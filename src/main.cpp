#include <Arduino.h>
#include "midi.h"
#include "display.h"
#include "config.h"
#include "uart.h"
#include "utils.h"
#include "switches.h"
#include "rgb_led_mux.h"

void setup() {
    // Initialize displays first to show loading screen
    initializeDisplays();

    // Show loading screen
    showLoadingScreen();

    // Initialize RGB LED multiplexer
    initializeRGBLEDMux();
    
    // LED startup sequence
    ledStartupSequence();

    // Initialize UART for host commands
    uart_init(UART_BAUD_RATE);

    // Initialize MIDI
    initializeMIDI();

    // Initialize footswitch pins
    initializeFootswitchPins();

    // Load configuration from flash
    loadConfigFromFlash();

    // Show normal displays after loading is complete
    drawConfigScreen();
    drawFootswitchScreen();

    printJsonLog("info", "App initialized");
}

void loop() {
    // Update RGB LED multiplexer (must be called frequently for smooth operation)
    ledMux.update();
    
    // Handle UART
    uart_loop();

    // Handle footswitch input and MIDI
    handleFootswitches();

    // Check timeout for configuring/loading messages
    if (isConfiguring && (millis() - configuringStartTime > 3000)) {
        hideConfiguringMessage();
    } else if (isLoading && (millis() - loadingStartTime > 2000)) {
        hideLoadingScreen();
    }

    // Small delay to avoid busy-loop
    delay(1);
}
