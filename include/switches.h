#ifndef SWITCHES_H
#define SWITCHES_H

#include <Arduino.h>
#include "midi.h" // for NUM_FOOTSWITCHES and FootswitchConfig

// Debounce delay (ms)
#define DEBOUNCE_DELAY 50

extern uint8_t currentSelectedFootswitch;

// Functions
void initializeFootswitchPins();
void handleFootswitches();

#endif // SWITCHES_H
