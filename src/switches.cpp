#include "switches.h"
#include "midi.h"
#include "utils.h"
#include "display.h"


// Footswitch state tracking
unsigned long lastDebounceTime = 0;
int8_t lastPressedFootswitch = -1;
uint8_t currentSelectedFootswitch = 0;

#define DIVIDER(R1) (4095 * 10000 / (R1 + 10000)) // Assuming R2 = 10k ohm
#define IN_RANGE(val, target, range) ((val) >= (target) - (range) && (val) <= (target) + (range))

void initializeFootswitchPins() {
    pinMode(FOOTSWITCH_LADDER_PIN, INPUT_PULLDOWN);
}

const int thresholds[NUM_FOOTSWITCHES] = {
    DIVIDER(0),
    DIVIDER(1000),
    DIVIDER(2200),
    DIVIDER(4700),
    DIVIDER(8200),
    DIVIDER(10000)
};

int8_t readFootswitchLadder() {
    int adcValue = analogRead(FOOTSWITCH_LADDER_PIN);

    for (uint8_t i = 0; i < NUM_FOOTSWITCHES; i++) {
        // if ADC value is within ±100 of threshold, return switch index
        if (IN_RANGE(adcValue, thresholds[i], 100)) {
            return i;
        }
    }
    return -1; // No footswitch pressed
}

void handleFootswitches() {
    int8_t pressedFootswitch = readFootswitchLadder();

    // Debounce logic
    if (pressedFootswitch != lastPressedFootswitch) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (pressedFootswitch != -1 && pressedFootswitch != currentSelectedFootswitch) {
            // Switch pressed
            currentSelectedFootswitch = pressedFootswitch;
            sendMidiCC(currentSelectedFootswitch);
            drawConfigScreen();
        }
    }

    lastPressedFootswitch = pressedFootswitch;
}