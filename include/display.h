#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "MultiTFT.hpp"
#include "midi.h"

// Display objects
extern MultiTFT footswitchDisplay;
extern MultiTFT configDisplay;

// Display function declarations
void initializeDisplays();
void updateFootswitchDisplay();
void updateConfigDisplay();
void drawFootswitchScreen();
void drawConfigScreen();
void showConfiguringMessage();
void hideConfiguringMessage();
void showLoadingScreen();

// Helper function for determining text color based on background brightness
uint16_t getTextColorForBackground(uint16_t backgroundColor);

#endif // DISPLAY_H