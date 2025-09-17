#include "config.h"
#include "utils.h"
#include "midi.h"
#include "switches.h"

// Preferences for storing configuration
Preferences preferences;

// UART communication buffer
String uartBuffer = "";
bool uartComplete = false;

void initializeDefaultConfig() {
    // Set default configuration for each footswitch with more descriptive names
    String defaultNames[] = {"CLEAN", "CRUNCH", "AMBIENT", "LOOP", "SOLO", "RHYTHM"};
    uint16_t defaultColors[] = {GREEN, RED, BLUE, MAGENTA, YELLOW, CYAN};
    
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        footswitches[i].name = defaultNames[i];
        footswitches[i].midiChannel = 1;
        footswitches[i].midiCC = 20 + i; // CC 20-25 by default
        footswitches[i].midiValue = 127;
        footswitches[i].enabled = true;
        footswitches[i].color = defaultColors[i];
    }
}

void saveConfigToFlash() {
    preferences.begin("midi-config", false);

    // Create JSON document
    JsonDocument doc;
    JsonArray switches = doc["switches"].to<JsonArray>();

    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        JsonObject sw = switches.add<JsonObject>();
        sw["id"] = i;
        sw["name"] = footswitches[i].name;
        sw["channel"] = footswitches[i].midiChannel;
        sw["cc"] = footswitches[i].midiCC;
        sw["value"] = footswitches[i].midiValue;
        sw["enabled"] = footswitches[i].enabled;
        sw["color"] = colorToHexString(footswitches[i].color);
    }

    String jsonString;
    serializeJson(doc, jsonString);

    preferences.putString("config", jsonString);
    preferences.end();

    printJsonLog("info", "Configuration saved to flash");
}

void loadConfigFromFlash() {
    preferences.begin("midi-config", true);
    String jsonString = preferences.getString("config", "");
    preferences.end();

    if (jsonString.length() == 0) {
        printJsonLog("warn", "No configuration found, using defaults");
        initializeDefaultConfig();
        saveConfigToFlash();
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) {
        printJsonLog("error", "Failed to parse configuration JSON, using defaults");
        initializeDefaultConfig();
        return;
    }

    JsonArray switches = doc["switches"];
    for (int i = 0; i < NUM_FOOTSWITCHES && i < switches.size(); i++) {
        JsonObject sw = switches[i];
        footswitches[i].name = sw["name"].as<String>();
        footswitches[i].midiChannel = sw["channel"];
        footswitches[i].midiCC = sw["cc"];
        footswitches[i].midiValue = sw["value"];
        footswitches[i].enabled = sw["enabled"];
        footswitches[i].color = hexStringToColor(sw["color"].as<String>());
    }

    printJsonLog("info", "Configuration loaded from flash");
}

void sendCurrentConfig() {
    JsonDocument doc;
    JsonArray switches = doc["switches"].to<JsonArray>();

    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        JsonObject sw = switches.add<JsonObject>();
        sw["id"] = i;
        sw["name"] = footswitches[i].name;
        sw["channel"] = footswitches[i].midiChannel;
        sw["cc"] = footswitches[i].midiCC;
        sw["value"] = footswitches[i].midiValue;
        sw["enabled"] = footswitches[i].enabled;
        sw["color"] = colorToHexString(footswitches[i].color);
    }

    doc["type"] = "config";
    doc["status"] = "success";

    String response;
    serializeJson(doc, response);
    Serial.println(response); // Already JSON, keep as is
}
