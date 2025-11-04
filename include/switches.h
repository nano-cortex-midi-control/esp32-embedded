#ifndef SWITCHES_H
#define SWITCHES_H

#include <Arduino.h>
#include "midi.h" // for NUM_FOOTSWITCHES and FootswitchConfig

// Debounce delay (ms)
#define DEBOUNCE_DELAY 50

// Calibration constants
#define CALIBRATION_SAMPLES 50        // Number of ADC samples to collect per button
#define CALIBRATION_MARGIN_PERCENT 15 // Margin percentage added to each side of the range

// Calibration data structure for each switch
struct SwitchCalibration {
    int minValue;  // Minimum ADC value for this switch
    int maxValue;  // Maximum ADC value for this switch
};

extern uint8_t currentSelectedFootswitch;
extern SwitchCalibration switchRanges[NUM_FOOTSWITCHES];

// Functions
void initializeFootswitchPins();
void handleFootswitches();

// Calibration functions
bool isCalibrationDataValid();
void saveCalibrationData();
void loadCalibrationData();
void performCalibration();
int readADCAverage(int samples);

#endif // SWITCHES_H
