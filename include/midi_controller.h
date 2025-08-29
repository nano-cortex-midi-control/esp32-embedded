#ifndef MIDI_CONTROLLER_H
#define MIDI_CONTROLLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Configuration constants
#define NUM_FOOTSWITCHES 6
#define DEBOUNCE_DELAY 50
#define MIDI_BAUD_RATE 31250
#define UART_BAUD_RATE 115200

// MIDI pins
#define MIDI_TX_PIN 17
#define MIDI_RX_PIN 16

// Display pins
#define TFT_CS1 5   // CS pin for first display (footswitch states)
#define TFT_CS2 15  // CS pin for second display (bank/config info)

// Color definitions
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Footswitch configuration structure
struct FootswitchConfig {
    String name;
    uint8_t midiChannel;
    uint8_t midiCC;
    uint8_t midiValue;
    bool enabled;
    uint16_t color;  // RGB565 color value
};

// External variable declarations
extern const int FOOTSWITCH_PINS[NUM_FOOTSWITCHES];
extern FootswitchConfig footswitches[NUM_FOOTSWITCHES];
extern bool footswitchStates[NUM_FOOTSWITCHES];
extern bool lastFootswitchStates[NUM_FOOTSWITCHES];
extern unsigned long lastDebounceTime[NUM_FOOTSWITCHES];

// Function declarations
void printJsonLog(const String &type, const String &message);
void initializeDefaultConfig();
void saveConfigToFlash();
void loadConfigFromFlash();
void sendCurrentConfig();
void processUartCommand(String command);
void sendMidiCC(int switchIndex);
void handleFootswitches();

// Display function declarations
void initializeDisplays();
void updateFootswitchDisplay();
void updateConfigDisplay();
void drawFootswitchScreen();
void drawConfigScreen();

#endif // MIDI_CONTROLLER_H
