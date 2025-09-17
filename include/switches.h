#ifndef SWITCHES_H
#define SWITCHES_H

#include <Arduino.h>
#include "midi.h" // for NUM_FOOTSWITCHES and FootswitchConfig

// Debounce delay (ms)
#define DEBOUNCE_DELAY 50

// Footswitch pin array and state tracking
extern const int FOOTSWITCH_PINS[NUM_FOOTSWITCHES];
extern bool footswitchStates[NUM_FOOTSWITCHES];
extern bool lastFootswitchStates[NUM_FOOTSWITCHES];
extern unsigned long lastDebounceTime[NUM_FOOTSWITCHES];

// Track currently selected footswitch (-1 if none)
extern int currentSelectedFootswitch;

// Track configuring state and timer
extern bool isConfiguring;
extern unsigned long configuringStartTime;

// Functions
void initializeFootswitchPins();
void handleFootswitches();

#endif // SWITCHES_H
