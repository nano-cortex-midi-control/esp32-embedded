#include <Arduino.h>
#include <ArduinoJson.h>
#include <MIDI.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include "midi_controller.h"
#include "MultiTFT.hpp"

// MIDI setup
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

// Preferences for storing configuration
Preferences preferences;

// Display setup
MultiTFT footswitchDisplay(TFT_CS1);  // Display for footswitch states
MultiTFT configDisplay(TFT_CS2);      // Display for bank/config info


// Pin definitions for footswitches (adjust based on your hardware)
const int FOOTSWITCH_PINS[NUM_FOOTSWITCHES] = {15, 4, 5, 18, 19, 21};

// LED pin for feedback (change if needed)
const int LED_PIN = 2;

// Blink patterns for commands
enum BlinkType {
    BLINK_GET_CONFIG,
    BLINK_SET_CONFIG,
    BLINK_TEST_SWITCH,
    BLINK_PING,
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
        case BLINK_PING:
            count = 1; onTime = 100; offTime = 100;
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

// Convert hex string to RGB565 color
uint16_t hexStringToColor(const String &hexStr) {
    String cleanHex = hexStr;
    if (cleanHex.startsWith("#")) {
        cleanHex = cleanHex.substring(1);
    }
    
    if (cleanHex.length() != 6) {
        return WHITE; // Default to white if invalid hex
    }
    
    // Convert hex string to long
    long hexValue = strtol(cleanHex.c_str(), NULL, 16);
    
    // Extract RGB components
    uint8_t r = (hexValue >> 16) & 0xFF;
    uint8_t g = (hexValue >> 8) & 0xFF;
    uint8_t b = hexValue & 0xFF;
    
    // Convert to RGB565
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Convert RGB565 color to hex string
String colorToHexString(uint16_t color) {
    // Convert RGB565 to RGB888
    uint8_t r = (color >> 8) & 0xF8;
    uint8_t g = (color >> 3) & 0xFC;
    uint8_t b = (color << 3) & 0xF8;
    
    // Format as hex string
    char hexStr[8];
    sprintf(hexStr, "#%02X%02X%02X", r, g, b);
    return String(hexStr);
}

void initializeDefaultConfig() {
    // Set default configuration for each footswitch with more descriptive names
    String defaultNames[] = {"CLEAN", "CRUNCH", "AMBIENT", "LOOP", "SOLO", "RHYTHM"};
    uint16_t defaultColors[] = {GREEN, RED, BLUE, MAGENTA, YELLOW, CYAN};
    
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        footswitches[i].name = defaultNames[i];
        footswitches[i].midiChannel = 1;
        footswitches[i].midiCC = 20 + i; // CC 20-25 by default
        footswitches[i].midiValue = 127;
        footswitches[i].enabled = true;
        footswitches[i].color = defaultColors[i];
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
        sw["color"] = colorToHexString(footswitches[i].color);
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
        footswitches[i].color = hexStringToColor(sw["color"].as<String>());
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
        sw["color"] = colorToHexString(footswitches[i].color);
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
            footswitches[i].color = hexStringToColor(sw["color"].as<String>());
        }

        saveConfigToFlash();
        blinkLed(BLINK_SET_CONFIG);
        updateFootswitchDisplay(); // Update display with new configuration
        updateConfigDisplay();     // Update config display as well
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
    else if (type == "ping") {
        blinkLed(BLINK_PING);
        printJsonLog("response", "Ping received");
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

// Display initialization
void initializeDisplays() {
    // Initialize both displays
    footswitchDisplay.begin(1); // Landscape
    configDisplay.begin(3);     // Landscape (rotated)
    
    // Draw initial screens
    drawFootswitchScreen();
    drawConfigScreen();
    
    printJsonLog("info", "Displays initialized");
}

// Draw the footswitch states screen (based on ST7796 current state screen)
void drawFootswitchScreen() {
    footswitchDisplay.select();
    footswitchDisplay.fillScreen(BLACK);
    footswitchDisplay.drawRect(0, 0, 480, 320, WHITE);

    // Define switch dimensions and positions for 2x3 grid (2 columns, 3 rows)
    int switch_width = 220;
    int switch_height = 90;
    int margin = 10;

    // Row Y positions (3 rows)
    int row1_y = 10;
    int row2_y = 115;
    int row3_y = 220;

    // Column X positions (2 columns)
    int left_col_x = 20;
    int right_col_x = 240;

    // === ROW 1 ===
    for (int i = 0; i < 2 && i < NUM_FOOTSWITCHES; i++) {
        int x = (i == 0) ? left_col_x : right_col_x;
        uint16_t bgColor = footswitches[i].enabled ? footswitches[i].color : BLACK;

        // Determine text color based on background brightness
        uint16_t textColor;
        if (footswitches[i].enabled) {
            // Check brightness by examining RGB components
            uint16_t color = footswitches[i].color;
            uint8_t r = (color >> 8) & 0xF8;
            uint8_t g = (color >> 3) & 0xFC;
            uint8_t b = (color << 3) & 0xF8;
            
            // Calculate brightness using standard formula
            uint16_t brightness = (r * 299 + g * 587 + b * 114) / 1000;
            
            // Use black text for bright colors, white text for dark colors
            if (brightness > 128) {
                textColor = BLACK;
            } else {
                textColor = WHITE;
            }
        } else {
            textColor = RED;
        }
        
        footswitchDisplay.fillRect(x, row1_y, switch_width, switch_height, bgColor);
        footswitchDisplay.drawRect(x, row1_y, switch_width, switch_height, WHITE);
        footswitchDisplay.setTextColor(textColor);
        footswitchDisplay.setTextSize(3);
        footswitchDisplay.setTextDatum(MC_DATUM);
        footswitchDisplay.drawString("[" + String(i + 1) + "] " + footswitches[i].name, 
                                   x + switch_width/2, row1_y + 30);
        footswitchDisplay.setTextSize(2);
        footswitchDisplay.drawString("CC" + String(footswitches[i].midiCC) + " Ch" + String(footswitches[i].midiChannel), 
                                   x + switch_width/2, row1_y + 65);
    }

    // === ROW 2 ===
    for (int i = 2; i < 4 && i < NUM_FOOTSWITCHES; i++) {
        int x = (i == 2) ? left_col_x : right_col_x;
        uint16_t bgColor = footswitches[i].enabled ? footswitches[i].color : BLACK;

        // Determine text color based on background brightness
        uint16_t textColor;
        if (footswitches[i].enabled) {
            // Check brightness by examining RGB components
            uint16_t color = footswitches[i].color;
            uint8_t r = (color >> 8) & 0xF8;
            uint8_t g = (color >> 3) & 0xFC;
            uint8_t b = (color << 3) & 0xF8;
            
            // Calculate brightness using standard formula
            uint16_t brightness = (r * 299 + g * 587 + b * 114) / 1000;
            
            // Use black text for bright colors, white text for dark colors
            if (brightness > 128) {
                textColor = BLACK;
            } else {
                textColor = WHITE;
            }
        } else {
            textColor = RED;
        }
        
        footswitchDisplay.fillRect(x, row2_y, switch_width, switch_height, bgColor);
        footswitchDisplay.drawRect(x, row2_y, switch_width, switch_height, WHITE);
        footswitchDisplay.setTextColor(textColor);
        footswitchDisplay.setTextSize(3);
        footswitchDisplay.setTextDatum(MC_DATUM);
        footswitchDisplay.drawString("[" + String(i + 1) + "] " + footswitches[i].name, 
                                   x + switch_width/2, row2_y + 30);
        footswitchDisplay.setTextSize(2);
        footswitchDisplay.drawString("CC" + String(footswitches[i].midiCC) + " Ch" + String(footswitches[i].midiChannel), 
                                   x + switch_width/2, row2_y + 65);
    }

    // === ROW 3 ===
    for (int i = 4; i < 6 && i < NUM_FOOTSWITCHES; i++) {
        int x = (i == 4) ? left_col_x : right_col_x;
        uint16_t bgColor = footswitches[i].enabled ? footswitches[i].color : BLACK;

        // Determine text color based on background brightness
        uint16_t textColor;
        if (footswitches[i].enabled) {
            // Check brightness by examining RGB components
            uint16_t color = footswitches[i].color;
            uint8_t r = (color >> 8) & 0xF8;
            uint8_t g = (color >> 3) & 0xFC;
            uint8_t b = (color << 3) & 0xF8;
            
            // Calculate brightness using standard formula
            uint16_t brightness = (r * 299 + g * 587 + b * 114) / 1000;
            
            // Use black text for bright colors, white text for dark colors
            if (brightness > 128) {
                textColor = BLACK;
            } else {
                textColor = WHITE;
            }
        } else {
            textColor = RED;
        }
        
        footswitchDisplay.fillRect(x, row3_y, switch_width, switch_height, bgColor);
        footswitchDisplay.drawRect(x, row3_y, switch_width, switch_height, WHITE);
        footswitchDisplay.setTextColor(textColor);
        footswitchDisplay.setTextSize(3);
        footswitchDisplay.setTextDatum(MC_DATUM);
        footswitchDisplay.drawString("[" + String(i + 1) + "] " + footswitches[i].name, 
                                   x + switch_width/2, row3_y + 30);
        footswitchDisplay.setTextSize(2);
        footswitchDisplay.drawString("CC" + String(footswitches[i].midiCC) + " Ch" + String(footswitches[i].midiChannel), 
                                   x + switch_width/2, row3_y + 65);
    }

    footswitchDisplay.deselect();
}

// Draw the configuration/bank screen (based on ST7796 bank list screen)
void drawConfigScreen() {
    configDisplay.select();
    configDisplay.fillScreen(BLACK);
    configDisplay.drawRect(0, 0, 480, 320, WHITE);

    // Title row (centered, big)
    configDisplay.setTextDatum(MC_DATUM);
    configDisplay.setTextColor(WHITE);
    configDisplay.setTextSize(4);
    configDisplay.drawString("MIDI CONTROLLER", 240, 50); // Centered at top

    // Configuration info rows
    configDisplay.setTextSize(3);
    configDisplay.setTextDatum(TL_DATUM);
    configDisplay.drawString("SWITCHES: " + String(NUM_FOOTSWITCHES), 20, 120);
    configDisplay.drawString("BAUD: " + String(MIDI_BAUD_RATE), 300, 120);

    // Status row
    configDisplay.setTextSize(2);
    configDisplay.setTextColor(GREEN);
    configDisplay.drawString("STATUS: READY", 20, 170);
    
    // UART commands info
    configDisplay.setTextColor(CYAN);
    configDisplay.drawString("UART Commands:", 20, 200);
    configDisplay.setTextSize(1);
    configDisplay.drawString("get_config, set_config, test_switch", 20, 220);

    // Navigation row (bottom)
    configDisplay.setTextSize(2);
    configDisplay.setTextColor(WHITE);
    configDisplay.setTextDatum(TL_DATUM);
    configDisplay.drawString("< CONFIG", 30, 260);
    configDisplay.setTextDatum(TR_DATUM);
    configDisplay.drawString("STATUS >", 450, 260);

    configDisplay.deselect();
}

// Update footswitch display when states change
void updateFootswitchDisplay() {
    drawFootswitchScreen();
}

// Update config display (can be called when configuration changes)
void updateConfigDisplay() {
    drawConfigScreen();
}

void handleFootswitches() {
    bool displayNeedsUpdate = false;
    
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
                displayNeedsUpdate = true;

                // Send MIDI on button press (LOW because of pull-up)
                if (footswitchStates[i] == LOW) {
                    sendMidiCC(i);
                }
            }
        }

        lastFootswitchStates[i] = reading;
    }
    
    // Update display if any footswitch state changed
    if (displayNeedsUpdate) {
        updateFootswitchDisplay();
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

    // Initialize displays
    initializeDisplays();

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
