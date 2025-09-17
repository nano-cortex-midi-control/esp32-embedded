#include "switches.h"
#include "midi.h"
#include "switches.h"
#include "midi.h"
#include "utils.h"
#include "display.h"

// Pin definitions for footswitches (adjust based on your hardware)
const int FOOTSWITCH_PINS[NUM_FOOTSWITCHES] = {13, 12, 14, 27, 26, 25};

// Footswitch states and configurations
bool footswitchStates[NUM_FOOTSWITCHES] = {HIGH}; // HIGH means not pressed (pull-up)
bool lastFootswitchStates[NUM_FOOTSWITCHES] = {HIGH}; // HIGH means not pressed (pull-up)
unsigned long lastDebounceTime[NUM_FOOTSWITCHES] = {0};
int currentSelectedFootswitch = 0;
bool isConfiguring = false;
unsigned long configuringStartTime = 0;

void initializeFootswitchPins() {
    // Initialize footswitch pins with pull-up resistors
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        pinMode(FOOTSWITCH_PINS[i], INPUT_PULLUP);
    }
    printJsonLog("info", "Footswitch pins initialized");
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
                // Button released (transition from LOW to HIGH)
                if (footswitchStates[i] == LOW && reading == HIGH) {
                    currentSelectedFootswitch = i;
                    displayNeedsUpdate = true;
                    sendMidiCC(i);
                }
                footswitchStates[i] = reading;
            }
        }

        lastFootswitchStates[i] = reading;
    }
    
    // Update display if any footswitch state changed (on release only)
    if (displayNeedsUpdate) {
        updateConfigDisplay();
    }
}