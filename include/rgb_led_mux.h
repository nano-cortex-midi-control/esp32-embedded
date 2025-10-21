#ifndef RGB_LED_MUX_H
#define RGB_LED_MUX_H

#include <Arduino.h>

// LED color structure
struct LEDColor {
    uint8_t red;
    uint8_t green; 
    uint8_t blue;
    
    LEDColor() : red(0), green(0), blue(0) {}
    LEDColor(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
};

// Predefined colors
extern const LEDColor COLOR_OFF;
extern const LEDColor COLOR_RED;
extern const LEDColor COLOR_GREEN;
extern const LEDColor COLOR_BLUE;
extern const LEDColor COLOR_YELLOW;
extern const LEDColor COLOR_MAGENTA;
extern const LEDColor COLOR_CYAN;
extern const LEDColor COLOR_WHITE;

// LED states
enum LEDState {
    LED_OFF,
    LED_ON,
    LED_BLINK_SLOW,
    LED_BLINK_FAST,
    LED_PULSE
};

// LED multiplexer class
class RGBLEDMultiplexer {
private:
    static const int NUM_LEDS = 6;  // Based on LED_EN_0_PIN and LED_EN_1_PIN
    
    int redPin;
    int greenPin;
    int bluePin;
    int enablePins[NUM_LEDS];
    
    LEDColor ledColors[NUM_LEDS];
    LEDState ledStates[NUM_LEDS];
    unsigned long lastBlinkTime[NUM_LEDS];
    bool blinkState[NUM_LEDS];
    unsigned long lastPulseTime[NUM_LEDS];
    uint8_t pulseDirection[NUM_LEDS]; // 0 = up, 1 = down
    uint8_t pulseBrightness[NUM_LEDS];
    
    int currentLED;
    unsigned long lastSwitchTime;
    static const unsigned long SWITCH_INTERVAL = 1; // 1ms per LED for smooth multiplexing
    static const unsigned long BLINK_SLOW_INTERVAL = 1000; // 1 second
    static const unsigned long BLINK_FAST_INTERVAL = 250;  // 250ms
    static const unsigned long PULSE_INTERVAL = 10; // 10ms for smooth pulse
    
    void selectLED(int ledIndex);
    void setRGBOutput(const LEDColor& color, uint8_t brightness = 255);
    void updateBlink(int ledIndex);
    void updatePulse(int ledIndex);
    
public:
    RGBLEDMultiplexer();
    
    // Initialize the multiplexer with pin assignments
    void begin(int redPin, int greenPin, int bluePin, int* enablePins);
    
    // Set LED color and state
    void setLED(int ledIndex, const LEDColor& color, LEDState state = LED_ON);
    void setLEDState(int ledIndex, LEDState state);
    void setLEDColor(int ledIndex, const LEDColor& color);
    
    // Get LED properties
    LEDColor getLEDColor(int ledIndex) const;
    LEDState getLEDState(int ledIndex) const;
    
    // Turn off all LEDs
    void allOff();
    
    // Turn on all LEDs with same color
    void allOn(const LEDColor& color, LEDState state = LED_ON);
    
    // Must be called in main loop for multiplexing and animations
    void update();
    
    // Get number of available LEDs
    int getNumLEDs() const { return NUM_LEDS; }
};

// Global LED multiplexer instance
extern RGBLEDMultiplexer ledMux;

// Initialization function
void initializeRGBLEDMux();

// Convenience functions for common LED operations
void setFootswitchLED(int footswitch, const LEDColor& color, LEDState state = LED_ON);
void setAllFootswitchLEDs(const LEDColor& color, LEDState state = LED_ON);
void turnOffAllLEDs();

// LED effect functions
void ledStartupSequence();
void ledMIDIActivity(bool isReceiving, bool isTransmitting);

#endif // RGB_LED_MUX_H
