#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "midi_controller.h"

// Function declarations for configuration management
void initializeDefaultConfig();
void saveConfigToFlash();
void loadConfigFromFlash();
void sendCurrentConfig();
void processUartCommand(String command);

// UART communication variables
extern String uartBuffer;
extern bool uartComplete;

#endif // CONFIG_H
