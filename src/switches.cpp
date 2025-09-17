// ...existing code...
#include "switches.h"
#include "midi.h"
#include "utils.h"
#include "display.h"

// Expected analog value ranges for each switch (tune these based on your resistor values)
const int FOOTSWITCH_THRESHOLDS[NUM_FOOTSWITCHES + 1] = {
    0,    // Min value for switch 0
    200,  // Min value for switch 1
    400,  // Min value for switch 2
    600,  // Min value for switch 3
    800,  // Min value for switch 4
    1000, // Min value for switch 5
    4096  // Max ADC value (ESP32 ADC is 12-bit)
};

// Footswitch state tracking
int lastPressedFootswitch = -1;
unsigned long lastDebounceTime = 0;
int currentSelectedFootswitch = -1;
bool isConfiguring = false;
unsigned long configuringStartTime = 0;

void initializeFootswitchPins() {
    pinMode(FOOTSWITCH_LADDER_PIN, INPUT);
    printJsonLog("info", "Footswitch resistor ladder initialized");
}

void handleFootswitches() {
    int analogValue = analogRead(FOOTSWITCH_LADDER_PIN);
    int pressedFootswitch = -1;

    // Determine which switch is pressed based on analog value
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        if (analogValue >= FOOTSWITCH_THRESHOLDS[i] && analogValue < FOOTSWITCH_THRESHOLDS[i + 1]) {
            pressedFootswitch = i;
            break;
        }
    }

    // Debounce logic
    if (pressedFootswitch != lastPressedFootswitch) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (pressedFootswitch != -1 && pressedFootswitch != currentSelectedFootswitch) {
            // Switch pressed
            currentSelectedFootswitch = pressedFootswitch;
            sendMidiCC(currentSelectedFootswitch);
            updateConfigDisplay();
        } else if (pressedFootswitch == -1 && currentSelectedFootswitch != -1) {
            // Switch released
            // currentSelectedFootswitch = -1;
            // updateConfigDisplay();
        }
    }

    lastPressedFootswitch = pressedFootswitch;
}