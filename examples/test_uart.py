#!/usr/bin/env python3
"""
ESP32 MIDI Footswitch Controller Test Script

This script demonstrates how to communicate with the ESP32 MIDI controller
over UART to read and modify the configuration.

The ESP32 now outputs JSON-formatted log messages and command responses:
- Log messages: {"type": "info|warn|error|midi", "message": "..."}
- Command responses: {"type": "config|response|error", ...}

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
    """Set an example configuration."""
    print("Setting example configuration...")
    
    example_config = {
        "type": "set_config",
        "switches": [
            {"id": 0, "name": "Distortion", "channel": 1, "cc": 20, "value": 127, "enabled": True},
            {"id": 1, "name": "Chorus", "channel": 1, "cc": 21, "value": 100, "enabled": True},
            {"id": 2, "name": "Reverb", "channel": 1, "cc": 22, "value": 80, "enabled": True},
            {"id": 3, "name": "Delay", "channel": 1, "cc": 23, "value": 90, "enabled": True},
            {"id": 4, "name": "Wah", "channel": 1, "cc": 24, "value": 127, "enabled": True},
            {"id": 5, "name": "Boost", "channel": 1, "cc": 25, "value": 127, "enabled": True}
        ]
    }
    
    response = send_command(ser, example_config)
    print(json.dumps(response, indent=2))

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
            print("2. Set example configuration")
            print("3. Test switch 0")
            print("4. Test switch 1")
            print("5. Test all switches")
            print("6. Exit")
            
            choice = input("\nEnter choice (1-6): ").strip()
            
            if choice == '1':
                get_config(ser)
            elif choice == '2':
                set_example_config(ser)
            elif choice == '3':
                test_switch(ser, 0)
            elif choice == '4':
                test_switch(ser, 1)
            elif choice == '5':
                for i in range(6):
                    test_switch(ser, i)
                    time.sleep(0.5)
            elif choice == '6':
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
