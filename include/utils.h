#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "midi_controller.h"  // For color definitions

// LED pin for feedback
extern const int LED_PIN;

// Blink patterns for commands
enum BlinkType {
    BLINK_GET_CONFIG,
    BLINK_SET_CONFIG,
    BLINK_TEST_SWITCH,
    BLINK_PING,
    BLINK_ERROR
};

// Utility function declarations
void printJsonLog(const String &type, const String &message);
void blinkLed(BlinkType type);
uint16_t hexStringToColor(const String &hexStr);
String colorToHexString(uint16_t color);

#endif // UTILS_H