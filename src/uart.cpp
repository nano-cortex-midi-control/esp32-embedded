#include "uart.h"
#include "config.h"
#include "utils.h"
#include "display.h"
#include "switches.h"

void uart_init(unsigned long baudRate) {
    Serial.begin(baudRate);
    printJsonLog("info", "UART initialized");
}

void uart_loop() {
    static String uartBuffer = "";
    static bool uartComplete = false;

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
        showConfiguringMessage(); // Show configuring message for 3 seconds
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