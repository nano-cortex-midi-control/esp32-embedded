#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "midi.h"

// Function declarations for configuration management
void initializeDefaultConfig();
void saveConfigToFlash();
void loadConfigFromFlash();
void sendCurrentConfig();

#endif // CONFIG_H
