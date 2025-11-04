#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "MultiTFT.hpp"
#include "midi.h"

// Display objects
extern MultiTFT footswitchDisplay;
extern MultiTFT configDisplay;

extern bool isConfiguring;
extern unsigned long configuringStartTime;
extern bool isLoading;
extern unsigned long loadingStartTime;

// Display function declarations
void initializeDisplays();
void drawFootswitchScreen();
void drawConfigScreen();
void showConfiguringMessage();
void hideConfiguringMessage();
void showLoadingScreen();
void hideLoadingScreen();

// Calibration display functions
void showCalibrationScreen(int buttonNum, bool success);
void showCalibrationWarning(const char* message);
void showCalibrationComplete();

// Helper function for determining text color based on background brightness
uint16_t getTextColorForBackground(uint16_t backgroundColor);

#endif // DISPLAY_H