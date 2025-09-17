#include "midi.h"
#include "utils.h"
#include "display.h"

// MIDI setup
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

// Footswitch configuration array is defined elsewhere (switches module)
FootswitchConfig footswitches[NUM_FOOTSWITCHES];

void initializeMIDI() {
    // Initialize MIDI on Serial2 (pins 16=RX, 17=TX)
    Serial2.begin(MIDI_BAUD_RATE, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
    MIDI.begin();
    printJsonLog("info", "MIDI initialized");
}

void sendMidiCC(int switchIndex) {
    if (switchIndex < 0 || switchIndex >= NUM_FOOTSWITCHES) return;
    if (!footswitches[switchIndex].enabled) return;

    MIDI.sendProgramChange(
        footswitches[switchIndex].midiCC,
        footswitches[switchIndex].midiChannel
    );

    printJsonLog("midi", "MIDI CC sent: Ch" + String(footswitches[switchIndex].midiChannel) +
        " CC" + String(footswitches[switchIndex].midiCC) +
        " Val" + String(footswitches[switchIndex].midiValue));
}