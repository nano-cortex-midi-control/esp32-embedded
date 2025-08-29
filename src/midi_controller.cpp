#include "midi_controller.h"
#include "utils.h"
#include "display.h"

// MIDI setup
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

// Pin definitions for footswitches (adjust based on your hardware)
const int FOOTSWITCH_PINS[NUM_FOOTSWITCHES] = {13, 12, 14, 27, 26, 25};

// Footswitch states and configurations
FootswitchConfig footswitches[NUM_FOOTSWITCHES];
bool footswitchStates[NUM_FOOTSWITCHES] = {false};
bool lastFootswitchStates[NUM_FOOTSWITCHES] = {false};
unsigned long lastDebounceTime[NUM_FOOTSWITCHES] = {0};
int currentSelectedFootswitch = -1;

void initializeMIDI() {
    // Initialize MIDI on Serial2 (pins 16=RX, 17=TX)
    Serial2.begin(31250, SERIAL_8N1, 16, 17);
    MIDI.begin();
    printJsonLog("info", "MIDI initialized");
}

void initializeFootswitchPins() {
    // Initialize footswitch pins with pull-up resistors
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        pinMode(FOOTSWITCH_PINS[i], INPUT_PULLUP);
    }
    printJsonLog("info", "Footswitch pins initialized");
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
        // updateFootswitchDisplay();
        updateConfigDisplay();
    }
}