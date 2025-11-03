#include "rgb_led_mux.h"

// Predefined colors
const LEDColor COLOR_OFF(0, 0, 0);
const LEDColor COLOR_RED(255, 0, 0);
const LEDColor COLOR_GREEN(0, 255, 0);
const LEDColor COLOR_BLUE(0, 0, 255);
const LEDColor COLOR_YELLOW(255, 255, 0);
const LEDColor COLOR_MAGENTA(255, 0, 255);
const LEDColor COLOR_CYAN(0, 255, 255);
const LEDColor COLOR_WHITE(255, 255, 255);

// Global LED multiplexer instance
RGBLEDMultiplexer ledMux;

RGBLEDMultiplexer::RGBLEDMultiplexer() 
    : redPin(-1), greenPin(-1), bluePin(-1), currentLED(0), lastSwitchTime(0) {
    
    // Initialize enable pins array
    for (int i = 0; i < NUM_LEDS; i++) {
        enablePins[i] = -1;
        ledColors[i] = COLOR_OFF;
        ledStates[i] = LED_OFF;
        lastBlinkTime[i] = 0;
        blinkState[i] = false;
        lastPulseTime[i] = 0;
        pulseDirection[i] = 0;
        pulseBrightness[i] = 0;
    }
}

void RGBLEDMultiplexer::begin(int redPin, int greenPin, int bluePin, int* enablePins) {
    this->redPin = redPin;
    this->greenPin = greenPin;
    this->bluePin = bluePin;
    
    // Copy enable pins
    for (int i = 0; i < NUM_LEDS; i++) {
        this->enablePins[i] = enablePins[i];
    }
    
    // Configure RGB pins as outputs
    pinMode(this->redPin, OUTPUT);
    pinMode(this->greenPin, OUTPUT);
    pinMode(this->bluePin, OUTPUT);
    
    // Configure enable pins as outputs
    for (int i = 0; i < NUM_LEDS; i++) {
        if (this->enablePins[i] >= 0) {
            pinMode(this->enablePins[i], OUTPUT);
            digitalWrite(this->enablePins[i], HIGH); // Disable all LEDs initially
        }
    }
    
    // Set initial RGB output to off
    setRGBOutput(COLOR_OFF);
}

void RGBLEDMultiplexer::selectLED(int ledIndex) {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return;
    
    // Disable all LEDs first
    for (int i = 0; i < NUM_LEDS; i++) {
        if (enablePins[i] >= 0) {
            digitalWrite(enablePins[i], HIGH);
        }
    }
    
    // Enable the selected LED
    if (enablePins[ledIndex] >= 0) {
        digitalWrite(enablePins[ledIndex], LOW);
    }
}

void RGBLEDMultiplexer::setRGBOutput(const LEDColor& color, uint8_t brightness) {
    // Apply brightness scaling
    int red = (color.red * brightness) / 255;
    int green = (color.green * brightness) / 255;
    int blue = (color.blue * brightness) / 255;
    
    // Invert values for common anode configuration (0 = ON, 255 = OFF)
    analogWrite(redPin, 255 - red);
    analogWrite(greenPin, 255 - green);
    analogWrite(bluePin, 255 - blue);
}

void RGBLEDMultiplexer::updateBlink(int ledIndex) {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return;
    
    unsigned long interval;
    switch (ledStates[ledIndex]) {
        case LED_BLINK_SLOW:
            interval = BLINK_SLOW_INTERVAL;
            break;
        case LED_BLINK_FAST:
            interval = BLINK_FAST_INTERVAL;
            break;
        default:
            return; // Not a blinking state
    }
    
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime[ledIndex] >= interval) {
        blinkState[ledIndex] = !blinkState[ledIndex];
        lastBlinkTime[ledIndex] = currentTime;
    }
}

void RGBLEDMultiplexer::updatePulse(int ledIndex) {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return;
    if (ledStates[ledIndex] != LED_PULSE) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastPulseTime[ledIndex] >= PULSE_INTERVAL) {
        if (pulseDirection[ledIndex] == 0) { // Brightness increasing
            pulseBrightness[ledIndex] += 5;
            if (pulseBrightness[ledIndex] >= 255) {
                pulseBrightness[ledIndex] = 255;
                pulseDirection[ledIndex] = 1; // Start decreasing
            }
        } else { // Brightness decreasing
            if (pulseBrightness[ledIndex] >= 5) {
                pulseBrightness[ledIndex] -= 5;
            } else {
                pulseBrightness[ledIndex] = 0;
                pulseDirection[ledIndex] = 0; // Start increasing
            }
        }
        lastPulseTime[ledIndex] = currentTime;
    }
}

void RGBLEDMultiplexer::setLED(int ledIndex, const LEDColor& color, LEDState state) {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return;
    
    ledColors[ledIndex] = color;
    ledStates[ledIndex] = state;
    
    // Reset animation states
    lastBlinkTime[ledIndex] = millis();
    blinkState[ledIndex] = true;
    lastPulseTime[ledIndex] = millis();
    pulseBrightness[ledIndex] = 0;
    pulseDirection[ledIndex] = 0;
}

void RGBLEDMultiplexer::setLEDState(int ledIndex, LEDState state) {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return;
    
    ledStates[ledIndex] = state;
    
    // Reset animation states
    lastBlinkTime[ledIndex] = millis();
    blinkState[ledIndex] = true;
    lastPulseTime[ledIndex] = millis();
    pulseBrightness[ledIndex] = 0;
    pulseDirection[ledIndex] = 0;
}

void RGBLEDMultiplexer::setLEDColor(int ledIndex, const LEDColor& color) {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return;
    ledColors[ledIndex] = color;
}

LEDColor RGBLEDMultiplexer::getLEDColor(int ledIndex) const {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return COLOR_OFF;
    return ledColors[ledIndex];
}

LEDState RGBLEDMultiplexer::getLEDState(int ledIndex) const {
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) return LED_OFF;
    return ledStates[ledIndex];
}

void RGBLEDMultiplexer::allOff() {
    for (int i = 0; i < NUM_LEDS; i++) {
        setLED(i, COLOR_OFF, LED_OFF);
    }
}

void RGBLEDMultiplexer::allOn(const LEDColor& color, LEDState state) {
    for (int i = 0; i < NUM_LEDS; i++) {
        setLED(i, color, state);
    } 
}

void RGBLEDMultiplexer::update() {
    unsigned long currentTime = millis();
    
    // Time to switch to next LED?
    if (currentTime - lastSwitchTime >= SWITCH_INTERVAL) {
        // Move to next LED
        currentLED = (currentLED + 1) % NUM_LEDS;
        lastSwitchTime = currentTime;
        
        // Update animations for current LED
        updateBlink(currentLED);
        updatePulse(currentLED);
        
        // Select the LED
        selectLED(currentLED);
        
        // Determine what to display
        LEDColor displayColor = COLOR_OFF;
        uint8_t brightness = 255;
        
        switch (ledStates[currentLED]) {
            case LED_OFF:
                displayColor = COLOR_OFF;
                break;
                
            case LED_ON:
                displayColor = ledColors[currentLED];
                break;
                
            case LED_BLINK_SLOW:
            case LED_BLINK_FAST:
                displayColor = blinkState[currentLED] ? ledColors[currentLED] : COLOR_OFF;
                break;
                
            case LED_PULSE:
                displayColor = ledColors[currentLED];
                brightness = pulseBrightness[currentLED];
                break;
        }
        
        // Set the RGB output
        setRGBOutput(displayColor, brightness);
    }
}

// Initialization function
void initializeRGBLEDMux() {
    // Pin assignments from platformio.ini
    int enablePins[] = {LED_EN_0_PIN, LED_EN_1_PIN, LED_EN_2_PIN, LED_EN_3_PIN, LED_EN_4_PIN, LED_EN_5_PIN};
    
    // Initialize the LED multiplexer
    ledMux.begin(LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN, enablePins);
    
    // Turn off all LEDs initially
    ledMux.allOff();
}

// Convenience functions for common LED operations
void setFootswitchLED(int footswitch, const LEDColor& color, LEDState state) {
    // Map footswitch number to LED index
    // This assumes footswitches 0-5 map to LEDs 0-1 in some pattern
    // Adjust this mapping based on your hardware design
    int ledIndex = footswitch % ledMux.getNumLEDs();
    ledMux.setLED(ledIndex, color, state);
}

void setAllFootswitchLEDs(const LEDColor& color, LEDState state) {
    ledMux.allOn(color, state);
}

void turnOffAllLEDs() {
    ledMux.allOff();
}

// LED effect functions
void ledStartupSequence() {
    // Simple startup sequence: cycle through colors
    ledMux.allOn(COLOR_RED, LED_ON);
    delay(200);
    ledMux.allOn(COLOR_GREEN, LED_ON);
    delay(200);
    ledMux.allOn(COLOR_BLUE, LED_ON);
    delay(200);
    ledMux.allOff();
}

void ledMIDIActivity(bool isReceiving, bool isTransmitting) {
    static unsigned long lastActivityTime = 0;
    static bool activityState = false;
    unsigned long currentTime = millis();
    
    // Flash LEDs briefly for MIDI activity
    if (isReceiving || isTransmitting) {
        LEDColor activityColor = isReceiving ? COLOR_GREEN : COLOR_BLUE;
        if (isReceiving && isTransmitting) {
            activityColor = COLOR_CYAN;
        }
        
        // Quick flash
        if (currentTime - lastActivityTime > 50) { // 50ms flash
            activityState = !activityState;
            lastActivityTime = currentTime;
            
            if (activityState) {
                ledMux.setLED(0, activityColor, LED_ON);
            } else {
                ledMux.setLED(0, COLOR_OFF, LED_OFF);
            }
        }
    }
}
