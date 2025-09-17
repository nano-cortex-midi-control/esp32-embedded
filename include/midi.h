#ifndef MIDI_CONTROLLER_H
#define MIDI_CONTROLLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <MIDI.h>
#include <HardwareSerial.h>

// Configuration constants
#define MIDI_BAUD_RATE 31250
#define UART_BAUD_RATE 115200

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

// Forward declaration for MIDI
extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDI;

// Footswitch configuration and runtime variables are provided by switches module
extern FootswitchConfig footswitches[NUM_FOOTSWITCHES];

// Function declarations
void initializeMIDI();
void sendMidiCC(int switchIndex);


#endif // MIDI_CONTROLLER_H
