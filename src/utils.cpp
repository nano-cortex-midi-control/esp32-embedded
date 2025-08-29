#include "utils.h"

// LED pin for feedback
const int LED_PIN = 2;

void printJsonLog(const String &type, const String &message) {
    JsonDocument doc;
    doc["type"] = type;
    doc["message"] = message;
    String output;
    serializeJson(doc, output);
    Serial.println(output);
}

void blinkLed(BlinkType type) {
    int count = 1;
    int onTime = 100;
    int offTime = 100;
    switch (type) {
        case BLINK_GET_CONFIG:
            count = 1; onTime = 100; offTime = 150;
            break;
        case BLINK_SET_CONFIG:
            count = 2; onTime = 100; offTime = 150;
            break;
        case BLINK_TEST_SWITCH:
            count = 1; onTime = 400; offTime = 200;
            break;
        case BLINK_PING:
            count = 1; onTime = 100; offTime = 100;
            break;
        case BLINK_ERROR:
            count = 3; onTime = 60; offTime = 80;
            break;
    }
    for (int i = 0; i < count; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(onTime);
        digitalWrite(LED_PIN, LOW);
        if (i < count - 1) delay(offTime);
    }
}

// Convert hex string to RGB565 color
uint16_t hexStringToColor(const String &hexStr) {
    String cleanHex = hexStr;
    if (cleanHex.startsWith("#")) {
        cleanHex = cleanHex.substring(1);
    }
    
    if (cleanHex.length() != 6) {
        return WHITE; // Default to white if invalid hex
    }
    
    // Convert hex string to long
    long hexValue = strtol(cleanHex.c_str(), NULL, 16);
    
    // Extract RGB components
    uint8_t r = (hexValue >> 16) & 0xFF;
    uint8_t g = (hexValue >> 8) & 0xFF;
    uint8_t b = hexValue & 0xFF;
    
    // Convert to RGB565
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Convert RGB565 color to hex string
String colorToHexString(uint16_t color) {
    // Convert RGB565 to RGB888
    uint8_t r = (color >> 8) & 0xF8;
    uint8_t g = (color >> 3) & 0xFC;
    uint8_t b = (color << 3) & 0xF8;
    
    // Format as hex string
    char hexStr[8];
    sprintf(hexStr, "#%02X%02X%02X", r, g, b);
    return String(hexStr);
}