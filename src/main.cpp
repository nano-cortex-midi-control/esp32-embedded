#include <Arduino.h>
#include <ArduinoJson.h>
#include <MIDI.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include "midi_controller.h"

// MIDI setup
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

// Preferences for storing configuration
Preferences preferences;


// Pin definitions for footswitches (adjust based on your hardware)
const int FOOTSWITCH_PINS[NUM_FOOTSWITCHES] = {15, 4, 5, 18, 19, 21};

// LED pin for feedback (change if needed)
const int LED_PIN = 2;

// Blink patterns for commands
enum BlinkType {
    BLINK_GET_CONFIG,
    BLINK_SET_CONFIG,
    BLINK_TEST_SWITCH,
    BLINK_ERROR
};

void blinkLed(BlinkType type) {
    int count = 1;
    int onTime = 100;
    int offTime = 100;
    switch (type) {
        case BLINK_GET_CONFIG:
            count = 1; onTime = 100; offTime = 150;
            break;
        case BLINK_SET_CONFIG:
            count = 2; onTime = 100; offTime = 150;
            break;
        case BLINK_TEST_SWITCH:
            count = 1; onTime = 400; offTime = 200;
            break;
        case BLINK_ERROR:
            count = 3; onTime = 60; offTime = 80;
            break;
    }
    for (int i = 0; i < count; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(onTime);
        digitalWrite(LED_PIN, LOW);
        if (i < count - 1) delay(offTime);
    }
}

// Footswitch states and configurations
FootswitchConfig footswitches[NUM_FOOTSWITCHES];
bool footswitchStates[NUM_FOOTSWITCHES] = {false};
bool lastFootswitchStates[NUM_FOOTSWITCHES] = {false};
unsigned long lastDebounceTime[NUM_FOOTSWITCHES] = {0};

// UART communication buffer
String uartBuffer = "";
bool uartComplete = false;

void printJsonLog(const String &type, const String &message) {
    JsonDocument doc;
    doc["type"] = type;
    doc["message"] = message;
    String output;
    serializeJson(doc, output);
    Serial.println(output);
}

void initializeDefaultConfig() {
    // Set default configuration for each footswitch
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        footswitches[i].name = "Switch " + String(i + 1);
        footswitches[i].midiChannel = 1;
        footswitches[i].midiCC = 20 + i; // CC 20-25 by default
        footswitches[i].midiValue = 127;
        footswitches[i].enabled = true;
    }
}

void saveConfigToFlash() {
    preferences.begin("midi-config", false);

    // Create JSON document
    JsonDocument doc;
    JsonArray switches = doc["switches"].to<JsonArray>();

    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        JsonObject sw = switches.add<JsonObject>();
        sw["id"] = i;
        sw["name"] = footswitches[i].name;
        sw["channel"] = footswitches[i].midiChannel;
        sw["cc"] = footswitches[i].midiCC;
        sw["value"] = footswitches[i].midiValue;
        sw["enabled"] = footswitches[i].enabled;
    }

    String jsonString;
    serializeJson(doc, jsonString);

    preferences.putString("config", jsonString);
    preferences.end();

    printJsonLog("info", "Configuration saved to flash");
}

void loadConfigFromFlash() {
    preferences.begin("midi-config", true);
    String jsonString = preferences.getString("config", "");
    preferences.end();

    if (jsonString.length() == 0) {
        printJsonLog("warn", "No configuration found, using defaults");
        initializeDefaultConfig();
        saveConfigToFlash();
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        printJsonLog("error", "Failed to parse configuration JSON, using defaults");
        initializeDefaultConfig();
        return;
    }

    JsonArray switches = doc["switches"];
    for (int i = 0; i < NUM_FOOTSWITCHES && i < switches.size(); i++) {
        JsonObject sw = switches[i];
        footswitches[i].name = sw["name"].as<String>();
        footswitches[i].midiChannel = sw["channel"];
        footswitches[i].midiCC = sw["cc"];
        footswitches[i].midiValue = sw["value"];
        footswitches[i].enabled = sw["enabled"];
    }

    printJsonLog("info", "Configuration loaded from flash");
}

void sendCurrentConfig() {
    JsonDocument doc;
    JsonArray switches = doc["switches"].to<JsonArray>();

    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        JsonObject sw = switches.add<JsonObject>();
        sw["id"] = i;
        sw["name"] = footswitches[i].name;
        sw["channel"] = footswitches[i].midiChannel;
        sw["cc"] = footswitches[i].midiCC;
        sw["value"] = footswitches[i].midiValue;
        sw["enabled"] = footswitches[i].enabled;
    }

    doc["type"] = "config";
    doc["status"] = "success";

    String response;
    serializeJson(doc, response);
    Serial.println(response); // Already JSON, keep as is
}

void processUartCommand(String command) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, command);

    if (error) {
        printJsonLog("error", "Invalid JSON");
        return;
    }

    String type = doc["type"];

    if (type == "get_config") {
        blinkLed(BLINK_GET_CONFIG);
        sendCurrentConfig();
    }
    else if (type == "set_config") {
        JsonArray switches = doc["switches"];

        if (switches.size() != NUM_FOOTSWITCHES) {
            blinkLed(BLINK_ERROR);
            printJsonLog("error", "Invalid number of switches");
            return;
        }

        // Update configuration
        for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
            JsonObject sw = switches[i];
            footswitches[i].name = sw["name"].as<String>();
            footswitches[i].midiChannel = sw["channel"];
            footswitches[i].midiCC = sw["cc"];
            footswitches[i].midiValue = sw["value"];
            footswitches[i].enabled = sw["enabled"];
        }

        saveConfigToFlash();
        blinkLed(BLINK_SET_CONFIG);
        printJsonLog("response", "Configuration updated");
    }
    else if (type == "test_switch") {
        int switchId = doc["switch_id"];
        if (switchId >= 0 && switchId < NUM_FOOTSWITCHES) {
            blinkLed(BLINK_TEST_SWITCH);
            sendMidiCC(switchId);
            printJsonLog("response", "Switch " + String(switchId + 1) + " tested");
        } else {
            blinkLed(BLINK_ERROR);
            printJsonLog("error", "Invalid switch ID");
        }
    }
    else {
        blinkLed(BLINK_ERROR);
        printJsonLog("error", "Unknown command type");
    }
}

void sendMidiCC(int switchIndex) {
    if (switchIndex < 0 || switchIndex >= NUM_FOOTSWITCHES) return;
    if (!footswitches[switchIndex].enabled) return;

    MIDI.sendControlChange(
        footswitches[switchIndex].midiCC,
        footswitches[switchIndex].midiValue,
        footswitches[switchIndex].midiChannel
    );

    printJsonLog("midi", "MIDI CC sent: Ch" + String(footswitches[switchIndex].midiChannel) +
        " CC" + String(footswitches[switchIndex].midiCC) +
        " Val" + String(footswitches[switchIndex].midiValue));
}

void handleFootswitches() {
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        int reading = digitalRead(FOOTSWITCH_PINS[i]);

        // Check if the switch state has changed
        if (reading != lastFootswitchStates[i]) {
            lastDebounceTime[i] = millis();
        }

        // Check if enough time has passed for debouncing
        if ((millis() - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
            // If the button state has changed
            if (reading != footswitchStates[i]) {
                footswitchStates[i] = reading;

                // Send MIDI on button press (LOW because of pull-up)
                if (footswitchStates[i] == LOW) {
                    sendMidiCC(i);
                }
            }
        }

        lastFootswitchStates[i] = reading;
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    printJsonLog("info", "MIDI Footswitch Controller Starting...");

    // Initialize MIDI on Serial2 (pins 16=RX, 17=TX)
    Serial2.begin(31250, SERIAL_8N1, 16, 17);
    MIDI.begin();

    // Initialize footswitch pins with pull-up resistors
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        pinMode(FOOTSWITCH_PINS[i], INPUT_PULLUP);
    }

    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Load configuration from flash
    loadConfigFromFlash();

    printJsonLog("info", "Ready for UART commands and footswitch input");
    printJsonLog("info", "Send JSON commands via UART for configuration");
}

void loop() {
    // Handle UART communication
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        if (inChar == '\n') {
            uartComplete = true;
        } else {
            uartBuffer += inChar;
        }
    }

    if (uartComplete) {
        uartBuffer.trim();
        if (uartBuffer.length() > 0) {
            processUartCommand(uartBuffer);
        }
        uartBuffer = "";
        uartComplete = false;
    }

    // Handle footswitch input
    handleFootswitches();

    // Small delay to prevent overwhelming the system
    delay(1);
}
