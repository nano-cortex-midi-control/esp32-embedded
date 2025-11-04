#include "switches.h"
#include "midi.h"
#include "utils.h"
#include "display.h"
#include <Preferences.h>

// Footswitch state tracking
unsigned long lastDebounceTime = 0;
int8_t lastPressedFootswitch = -1;
uint8_t currentSelectedFootswitch = 0;

#define R2 1000 // Resistor R2 in the voltage divider (ohms). Set to 1k (1000 ohm).
#define DIVIDER(R1) ((4095 * (R2)) / ((R1) + (R2))) // voltage divider calculation for 12-bit ADC

// Calibration ranges for each switch (min/max ADC values)
SwitchCalibration switchRanges[NUM_FOOTSWITCHES] = {
    {4000, 4095},  // Default for button 1
    {3750, 3950},  // Default for button 2
    {3250, 3550},  // Default for button 3
    {2850, 3150},  // Default for button 4
    {1750, 2050},  // Default for button 5
    {1100, 1400}   // Default for button 6
};

void initializeFootswitchPins() {
    pinMode(FOOTSWITCH_LADDER_PIN, INPUT_PULLDOWN);
}

// Read ADC value with averaging to reduce noise
int readADCAverage(int samples) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(FOOTSWITCH_LADDER_PIN);
        delay(5); // Small delay between samples
    }
    return sum / samples;
}

// Check if valid calibration data exists in flash
bool isCalibrationDataValid() {
    Preferences prefs;
    prefs.begin("footswitch-cal", true); // Read-only
    bool valid = prefs.getBool("valid", false);
    prefs.end();
    return valid;
}

// Save calibration data to flash
void saveCalibrationData() {
    Preferences prefs;
    prefs.begin("footswitch-cal", false); // Read-write
    
    // Save each switch's min/max values
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        String minKey = "min" + String(i);
        String maxKey = "max" + String(i);
        prefs.putInt(minKey.c_str(), switchRanges[i].minValue);
        prefs.putInt(maxKey.c_str(), switchRanges[i].maxValue);
    }
    
    // Mark calibration as valid
    prefs.putBool("valid", true);
    prefs.end();
    
    printJsonLog("info", "Calibration data saved to flash");
}

// Load calibration data from flash
void loadCalibrationData() {
    if (!isCalibrationDataValid()) {
        printJsonLog("warn", "No valid calibration data found, using defaults");
        return;
    }
    
    Preferences prefs;
    prefs.begin("footswitch-cal", true); // Read-only
    
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        String minKey = "min" + String(i);
        String maxKey = "max" + String(i);
        switchRanges[i].minValue = prefs.getInt(minKey.c_str(), switchRanges[i].minValue);
        switchRanges[i].maxValue = prefs.getInt(maxKey.c_str(), switchRanges[i].maxValue);
    }
    
    prefs.end();
    printJsonLog("info", "Calibration data loaded from flash");
}

// Perform calibration routine
void performCalibration() {
    printJsonLog("info", "Starting footswitch calibration");
    
    struct ButtonData {
        int centerValue;
        int minSample;
        int maxSample;
        bool calibrated;
    };
    
    ButtonData buttonData[NUM_FOOTSWITCHES];
    
    // Initialize all buttons as not calibrated
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        buttonData[i].calibrated = false;
    }
    
    // Collect ADC values for each button
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        showCalibrationScreen(i, false);
        
        // Wait for button press (ADC value to change significantly)
        int baselineADC = readADCAverage(10);
        
        bool buttonDetected = false;
        
        while (!buttonDetected) {
            int currentADC = analogRead(FOOTSWITCH_LADDER_PIN);
            
            // Detect significant change indicating button press
            if (abs(currentADC - baselineADC) > 200) {
                delay(100); // Wait for stabilization
                
                // Collect multiple samples to find min/max during button hold
                int minVal = 4095;
                int maxVal = 0;
                long sum = 0;
                int sampleCount = CALIBRATION_SAMPLES;
                
                for (int j = 0; j < sampleCount; j++) {
                    int sample = analogRead(FOOTSWITCH_LADDER_PIN);
                    if (sample < minVal) minVal = sample;
                    if (sample > maxVal) maxVal = sample;
                    sum += sample;
                    delay(5);
                }
                
                int centerValue = sum / sampleCount;
                
                // Check if this button was already calibrated
                bool alreadyCalibrated = false;
                for (int k = 0; k < i; k++) {
                    if (buttonData[k].calibrated) {
                        // Check if the current reading is close to a previously calibrated button
                        int diff = abs(centerValue - buttonData[k].centerValue);
                        if (diff < 150) {  // Within ~150 ADC units means same button
                            alreadyCalibrated = true;
                            String buttonNames[] = {"TOP LEFT", "TOP MIDDLE", "TOP RIGHT", 
                                                   "BOTTOM LEFT", "BOTTOM MIDDLE", "BOTTOM RIGHT"};
                            String msg = buttonNames[k] + " already calibrated!";
                            showCalibrationWarning(msg.c_str());
                            printJsonLog("warn", "Button " + String(k) + " already pressed");
                            delay(2000);
                            showCalibrationScreen(i, false);  // Show the prompt again
                            break;
                        }
                    }
                }
                
                // If not already calibrated, store the data
                if (!alreadyCalibrated) {
                    buttonData[i].centerValue = centerValue;
                    buttonData[i].minSample = minVal;
                    buttonData[i].maxSample = maxVal;
                    buttonData[i].calibrated = true;
                    
                    showCalibrationScreen(i, true);
                    delay(500);
                    
                    // Wait for button release
                    while (abs(analogRead(FOOTSWITCH_LADDER_PIN) - baselineADC) > 100) {
                        delay(50);
                    }
                    delay(200); // Debounce
                    buttonDetected = true;
                }
                // If already calibrated, wait for button release and continue waiting
                else {
                    while (abs(analogRead(FOOTSWITCH_LADDER_PIN) - baselineADC) > 100) {
                        delay(50);
                    }
                    delay(200); // Debounce
                    // Don't set buttonDetected, so we stay in the loop
                }
            }
            delay(10);
        }
    }
    
    // Sort button data by center value (descending order)
    for (int i = 0; i < NUM_FOOTSWITCHES - 1; i++) {
        for (int j = i + 1; j < NUM_FOOTSWITCHES; j++) {
            if (buttonData[j].centerValue > buttonData[i].centerValue) {
                ButtonData temp = buttonData[i];
                buttonData[i] = buttonData[j];
                buttonData[j] = temp;
            }
        }
    }
    
    // Calculate ranges with margins for each button
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        int center = buttonData[i].centerValue;
        int rawRange = buttonData[i].maxSample - buttonData[i].minSample;
        
        // Add margin percentage to the observed range
        int margin = (rawRange * CALIBRATION_MARGIN_PERCENT) / 100;
        if (margin < 50) margin = 50; // Minimum margin of 50 ADC units
        
        switchRanges[i].minValue = buttonData[i].minSample - margin;
        switchRanges[i].maxValue = buttonData[i].maxSample + margin;
        
        // Clamp to valid ADC range
        if (switchRanges[i].minValue < 0) switchRanges[i].minValue = 0;
        if (switchRanges[i].maxValue > 4095) switchRanges[i].maxValue = 4095;
    }
    
    // Check and fix overlaps between adjacent ranges
    for (int i = 0; i < NUM_FOOTSWITCHES - 1; i++) {
        // If current range's min overlaps with next range's max, split the difference
        if (switchRanges[i].minValue <= switchRanges[i + 1].maxValue) {
            int midpoint = (switchRanges[i].minValue + switchRanges[i + 1].maxValue) / 2;
            switchRanges[i].minValue = midpoint + 1;
            switchRanges[i + 1].maxValue = midpoint;
            
            printJsonLog("warn", "Overlap detected between buttons " + String(i) + 
                        " and " + String(i + 1) + ", adjusting ranges");
        }
    }
    
    // Validate and log final ranges
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        String msg = "Button " + String(i) + " range: " + 
                    String(switchRanges[i].minValue) + "-" + 
                    String(switchRanges[i].maxValue);
        printJsonLog("info", msg);
        
        // Ensure min < max
        if (switchRanges[i].minValue >= switchRanges[i].maxValue) {
            printJsonLog("error", "Invalid range for button " + String(i));
            // Fix by adding small range around center
            int center = buttonData[i].centerValue;
            switchRanges[i].minValue = center - 100;
            switchRanges[i].maxValue = center + 100;
        }
    }
    
    // Save to flash
    saveCalibrationData();
    
    // Show completion
    showCalibrationComplete();
    delay(2000);
    
    printJsonLog("info", "Calibration complete");
}

int8_t readFootswitchLadder() {
    int adcValue = analogRead(FOOTSWITCH_LADDER_PIN);

    // Check each switch range to see if ADC value falls within it
    for (uint8_t i = 0; i < NUM_FOOTSWITCHES; i++) {
        if (adcValue >= switchRanges[i].minValue && adcValue <= switchRanges[i].maxValue) {
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