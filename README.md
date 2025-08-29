# ESP32 MIDI Footswitch Controller

A configurable 6-footswitch MIDI controller for ESP32 with dual TFT displays and JSON-based configuration over UART.

## Features

- 6 configurable footswitches with debouncing
- TRS MIDI output on Serial2 (pins 16/17)
- Dual TFT displays for footswitch states and configuration info
- JSON configuration storage in flash memory
- UART communication for configuration editing
- Individual switch enable/disable
- Configurable MIDI channel, CC number, and value per switch
- Color-coded footswitch display
- LED feedback for command confirmation

## Project Structure

The project has been organized into multiple modules for better maintainability:

- `main.cpp` - Main setup and loop
- `midi_controller.cpp` - MIDI and footswitch handling
- `config.cpp` - Configuration management and UART commands
- `display.cpp` - TFT display functions
- `utils.cpp` - Utility functions (logging, LED, colors)

See [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) for detailed documentation.

## Hardware Setup

### Footswitch Connections
- Switch 1: GPIO 13
- Switch 2: GPIO 12  
- Switch 3: GPIO 14
- Switch 4: GPIO 27
- Switch 5: GPIO 26
- Switch 6: GPIO 25

Connect switches between GPIO pins and GND (internal pull-up resistors are enabled).

### MIDI Output
- MIDI TX: GPIO 17 (Serial2)
- MIDI RX: GPIO 16 (Serial2) - not used but available
- Use standard MIDI TRS wiring (31.25kbaud)

### TFT Displays
- Display 1 (Footswitch states): CS on GPIO 5
- Display 2 (Configuration info): CS on GPIO 15

### LED Feedback
- Status LED: GPIO 2

## UART Communication Protocol

### Get Current Configuration
```json
{"type": "get_config"}
```

**Response:**
```json
{
  "type": "config",
  "status": "success",
  "switches": [
    {
      "id": 0,
          "name": "Switch 1",
          "channel": 1,
          "cc": 20,
          "value": 127,
          "enabled": true
        },
        ...
      ]
    }
    ```

### Update Configuration
    ```json
    {
      "type": "set_config",
      "switches": [
        {
          "id": 0,
          "name": "Distortion",
          "channel": 1,
          "cc": 20,
          "value": 127,
          "enabled": true
        },
        {
          "id": 1,
          "name": "Chorus",
          "channel": 1,
          "cc": 21,
          "value": 100,
          "enabled": true
        },
        ...
      ]
    }
    ```

**Response:**
    ```json
    {"type": "response", "status": "success", "message": "Configuration updated"}
    ```

### Test Switch
    ```json
    {"type": "test_switch", "switch_id": 0}
    ```

**Response:**
    ```json
    {"type": "response", "status": "success", "message": "Switch 1 tested"}
    ```

### Error Response
    ```json
    {"type": "error", "message": "Error description"}
    ```

## Default Configuration

    Each switch starts with:
    - Name: "Switch X" (where X is 1-6)
    - MIDI Channel: 1
    - MIDI CC: 20-25 (switch 1-6 respectively)
    - MIDI Value: 127
    - Enabled: true

## Building and Uploading

    1. Install PlatformIO
    2. Open project folder
    3. Build and upload:
       ```bash
       pio run -t upload
       ```

## Serial Monitor

    Monitor serial output:
    ```bash
    pio device monitor
    ```

## App Integration

    Your colleague's app should:
    1. Connect to the ESP32 via USB serial at 115200 baud
    2. Send `{"type": "get_config"}` to retrieve current settings
    3. Allow user to modify switch configurations
    4. Send updated configuration using `{"type": "set_config", "switches": [...]}` 
    5. Optionally test switches using `{"type": "test_switch", "switch_id": X}`

## Future Enhancements

    - SPI screen support (to be added)
    - Additional MIDI message types (Program Change, Note On/Off)
    - Switch momentary/latching modes
    - LED indicators
    - Expression pedal inputs