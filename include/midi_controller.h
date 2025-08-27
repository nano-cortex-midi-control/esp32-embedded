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

// Footswitch configuration structure
struct FootswitchConfig {
    String name;
    uint8_t midiChannel;
    uint8_t midiCC;
    uint8_t midiValue;
    bool enabled;
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

#endif // MIDI_CONTROLLER_H
