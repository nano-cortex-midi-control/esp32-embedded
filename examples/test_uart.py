#!/usr/bin/env python3
"""
ESP32 MIDI Footswitch Controller Test Script

This script demonstrates how to communicate with the ESP32 MIDI controller
over UART to read and modify the configuration, including color settings for the displays.

The ESP32 now outputs JSON-formatted log messages and command responses:
- Log messages: {"type": "info|warn|error|midi", "message": "..."}
- Command responses: {"type": "config|response|error", ...}

Configuration includes:
- Switch names, MIDI channels, CC numbers, values, enabled state
- Custom colors for each switch (hex format: #RRGGBB)

Usage:
    python test_uart.py [port]

Example:
    python test_uart.py /dev/ttyUSB0
    python test_uart.py COM3  # Windows
"""

import serial
import json
import sys
import time

def send_command(ser, command):
    """Send a JSON command to the ESP32 and return the response."""
    json_str = json.dumps(command) + '\n'
    ser.write(json_str.encode())
    
    # Wait for response - read multiple lines until we get a command response
    max_attempts = 10
    attempt = 0
    
    while attempt < max_attempts:
        response = ser.readline().decode().strip()
        if not response:
            attempt += 1
            continue
            
        try:
            parsed_response = json.loads(response)
            
            # Check if this is a command response (not a log message)
            response_type = parsed_response.get("type", "")
            
            # Command responses have type "config", "response", or "error"
            # Log messages have type "info", "warn", "error", "midi"
            if response_type in ["config", "response"] or (response_type == "error" and "message" in parsed_response and "raw" not in parsed_response):
                return parsed_response
            else:
                # This is a log message, print it and continue waiting
                print(f"[ESP32 LOG] {response}")
                
        except json.JSONDecodeError:
            return {"error": "Invalid JSON response", "raw": response}
        
        attempt += 1
    
    return {"error": "No valid response received", "attempts": attempt}

def get_config(ser):
    """Get the current configuration from the ESP32."""
    print("Getting current configuration...")
    response = send_command(ser, {"type": "get_config"})
    print(json.dumps(response, indent=2))
    return response

def set_example_config(ser):
    """Set an example configuration with colors."""
    print("Setting example configuration...")
    
    example_config = {
        "type": "set_config",
        "switches": [
            {"id": 0, "name": "Distortion", "channel": 1, "cc": 20, "value": 127, "enabled": True, "color": "#FF4500"},
            {"id": 1, "name": "Chorus", "channel": 1, "cc": 21, "value": 100, "enabled": True, "color": "#FF4500"},
            {"id": 2, "name": "Reverb", "channel": 1, "cc": 22, "value": 80, "enabled": True, "color": "#FF4500"},
            {"id": 3, "name": "Delay", "channel": 1, "cc": 23, "value": 90, "enabled": True, "color": "#FF4500"},
            {"id": 4, "name": "Wah", "channel": 1, "cc": 24, "value": 127, "enabled": True, "color": "#FF4500"},
            {"id": 5, "name": "Boost", "channel": 1, "cc": 25, "value": 127, "enabled": True, "color": "#FF4500"}
        ]
    }
    
    response = send_command(ser, example_config)
    print(json.dumps(response, indent=2))

def set_guitar_preset_config(ser):
    """Set a guitar-themed configuration with appropriate colors."""
    print("Setting guitar preset configuration...")
    
    guitar_config = {
        "type": "set_config",
        "switches": [
            {"id": 0, "name": "CLEAN", "channel": 1, "cc": 20, "value": 127, "enabled": True, "color": "#00FF00"},    # Green
            {"id": 1, "name": "CRUNCH", "channel": 1, "cc": 21, "value": 127, "enabled": True, "color": "#FF8000"},   # Orange
            {"id": 2, "name": "LEAD", "channel": 1, "cc": 22, "value": 127, "enabled": True, "color": "#FF0000"},     # Red
            {"id": 3, "name": "SOLO", "channel": 1, "cc": 23, "value": 127, "enabled": True, "color": "#FFFF00"},     # Yellow
            {"id": 4, "name": "REVERB", "channel": 1, "cc": 24, "value": 80, "enabled": True, "color": "#0080FF"},    # Blue
            {"id": 5, "name": "DELAY", "channel": 1, "cc": 25, "value": 90, "enabled": True, "color": "#FF00FF"}      # Magenta
        ]
    }
    
    response = send_command(ser, guitar_config)
    print(json.dumps(response, indent=2))

def set_custom_color_config(ser):
    """Set a configuration with custom colors."""
    print("Setting custom color configuration...")
    
    # Get custom input from user
    print("\nEnter custom colors for each switch (or press Enter for default):")
    switches = []
    default_names = ["CLEAN", "CRUNCH", "AMBIENT", "LOOP", "SOLO", "RHYTHM"]
    default_colors = ["#00FF00", "#FF0000", "#0000FF", "#FF00FF", "#FFFF00", "#00FFFF"]
    
    for i in range(6):
        print(f"\nSwitch {i+1} ({default_names[i]}):")
        name = input(f"  Name (default: {default_names[i]}): ").strip() or default_names[i]
        color = input(f"  Color hex (default: {default_colors[i]}): ").strip() or default_colors[i]
        cc = input(f"  MIDI CC (default: {20+i}): ").strip()
        cc = int(cc) if cc else 20+i
        
        switches.append({
            "id": i,
            "name": name,
            "channel": 1,
            "cc": cc,
            "value": 127,
            "enabled": True,
            "color": color
        })
    
    custom_config = {
        "type": "set_config",
        "switches": switches
    }
    
    response = send_command(ser, custom_config)
    print(json.dumps(response, indent=2))

def show_color_examples():
    """Display common color examples for reference."""
    print("\nCommon Color Examples (Hex Format):")
    print("=" * 40)
    print("Red:     #FF0000    Bright Red")
    print("Green:   #00FF00    Bright Green") 
    print("Blue:    #0000FF    Bright Blue")
    print("Yellow:  #FFFF00    Bright Yellow")
    print("Cyan:    #00FFFF    Bright Cyan")
    print("Magenta: #FF00FF    Bright Magenta")
    print("Orange:  #FF8000    Orange")
    print("Purple:  #8000FF    Purple")
    print("Pink:    #FF0080    Hot Pink")
    print("Lime:    #80FF00    Lime Green")
    print("White:   #FFFFFF    White")
    print("Gray:    #808080    Gray")
    print("Black:   #000000    Black (off state)")
    print("=" * 40)

def test_switch(ser, switch_id):
    """Test a specific switch."""
    print(f"Testing switch {switch_id}...")
    response = send_command(ser, {"type": "test_switch", "switch_id": switch_id})
    print(json.dumps(response, indent=2))
    
    # Also listen for any MIDI log messages that might follow
    time.sleep(0.5)  # Give time for any log messages
    while ser.in_waiting > 0:
        line = ser.readline().decode().strip()
        if line:
            try:
                parsed = json.loads(line)
                if parsed.get("type") == "midi":
                    print(f"[MIDI LOG] {line}")
            except json.JSONDecodeError:
                print(f"[RAW] {line}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python test_uart.py <port>")
        print("Example: python test_uart.py /dev/ttyUSB0")
        return
    
    port = sys.argv[1]
    
    try:
        # Open serial connection
        ser = serial.Serial(port, 115200, timeout=2)
        time.sleep(2)  # Wait for ESP32 to initialize
        
        print(f"Connected to {port}")
        print("=" * 50)
        
        # Read any initial log messages from ESP32 startup
        print("Reading initial ESP32 messages...")
        start_time = time.time()
        while time.time() - start_time < 3:  # Read for 3 seconds
            if ser.in_waiting > 0:
                line = ser.readline().decode().strip()
                if line:
                    try:
                        parsed = json.loads(line)
                        print(f"[ESP32 LOG] {line}")
                    except json.JSONDecodeError:
                        print(f"[ESP32 RAW] {line}")
            else:
                time.sleep(0.1)
        
        print("=" * 50)
        
        while True:
            print("\nESP32 MIDI Controller Test Menu:")
            print("1. Get current configuration")
            print("2. Set example configuration (with colors)")
            print("3. Set guitar preset configuration")
            print("4. Set custom color configuration")
            print("5. Show color examples")
            print("6. Test switch 0")
            print("7. Test switch 1")
            print("8. Test all switches")
            print("9. Exit")
            
            choice = input("\nEnter choice (1-9): ").strip()
            
            if choice == '1':
                get_config(ser)
            elif choice == '2':
                set_example_config(ser)
            elif choice == '3':
                set_guitar_preset_config(ser)
            elif choice == '4':
                set_custom_color_config(ser)
            elif choice == '5':
                show_color_examples()
            elif choice == '6':
                test_switch(ser, 0)
            elif choice == '7':
                test_switch(ser, 1)
            elif choice == '8':
                for i in range(6):
                    test_switch(ser, i)
                    time.sleep(0.5)
            elif choice == '9':
                break
            else:
                print("Invalid choice")
                
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        if 'ser' in locals():
            ser.close()

if __name__ == "__main__":
    main()
