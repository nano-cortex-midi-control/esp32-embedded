#include <Arduino.h>
#include "midi_controller.h"
#include "display.h"
#include "config.h"
#include "utils.h"

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    printJsonLog("info", "MIDI Footswitch Controller Starting...");

    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize MIDI
    initializeMIDI();

    // Initialize footswitch pins
    initializeFootswitchPins();

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
