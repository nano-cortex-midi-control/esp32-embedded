#include <Arduino.h>
#include "midi.h"
#include "display.h"
#include "config.h"
#include "uart.h"
#include "utils.h"
#include "switches.h"

void setup() {
    // LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize displays first to show loading screen
    initializeDisplays();
    // showLoadingScreen();

    // Initialize UART for host commands
    uart_init(UART_BAUD_RATE);

    // Initialize MIDI
    initializeMIDI();

    // Initialize footswitch pins
    initializeFootswitchPins();

    // Load configuration from flash
    loadConfigFromFlash();

    // Show normal displays after loading is complete
    updateConfigDisplay();
    updateFootswitchDisplay();

    printJsonLog("info", "App initialized");
}

void loop() {
    // Handle UART
    uart_loop();

    // Handle footswitch input and MIDI
    handleFootswitches();

    // Check configuring message timeout
    if (isConfiguring && (millis() - configuringStartTime > 3000)) {
        hideConfiguringMessage();
    }

    // Small delay to avoid busy-loop
    delay(1);
}
