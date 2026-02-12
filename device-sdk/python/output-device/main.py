#!/usr/bin/env python3
"""
atCloud365 Output Device - Python SDK Example

This example demonstrates how to:
1. Authenticate with atCloud365 platform via HTTPS POST
2. Connect to Socket.IO for real-time communication
3. Receive control commands from platform
4. Control outputs (simulated GPIO/relay control)
5. Send status feedback to platform

Author: atCloud365
Date: 2026-02-12
"""

import sys
import time
import json
import requests
import socketio
from urllib.parse import urlencode
from typing import List, Dict, Any, Optional
import threading
import signal

# Import configuration
try:
    from config import (
        DEVICE_SN,
        CLIENT_SECRET_KEY,
        SERVER_URL,
        SERVER_PORT,
        API_PATH,
        DEVICE_AUTH_URI,
        SENSOR_IDS,
        STATUS_REPORT_INTERVAL,
        BLINK_INTERVAL,
        DEBUG_ENABLED
    )
except ImportError:
    print("❌ Error: config.py not found!")
    print("Please copy config.example.py to config.py and configure it.")
    sys.exit(1)

# ==================================================
# Global Variables
# ==================================================
sio = socketio.Client(
    reconnection=True,
    reconnection_attempts=50,
    reconnection_delay=5,
    logger=DEBUG_ENABLED,
    engineio_logger=DEBUG_ENABLED
)

auth_token: Optional[str] = None
socket_sid: Optional[str] = None
bootup_ready: bool = False
last_status_report: float = 0
state_changed: bool = False
running: bool = True

# Output states
class OutputPin:
    def __init__(self, index: int, name: str):
        self.index = index
        self.name = name
        self.state = False
        self.blink_count = 0
        
output_pins: List[OutputPin] = [
    OutputPin(0, f"Output-{SENSOR_IDS[i]}") 
    for i in range(len(SENSOR_IDS))
]

last_blink_toggle: float = 0

# ==================================================
# Utility Functions
# ==================================================
def debug_print(message: str):
    """Print debug message if debug is enabled."""
    if DEBUG_ENABLED:
        print(message)

def get_current_time() -> float:
    """Get current time in seconds."""
    return time.time()

# ==================================================
# HTTP Authentication
# ==================================================
def authenticate_device() -> Optional[str]:
    """
    Authenticate device with atCloud365 platform.
    
    Returns:
        str: Authentication token if successful, None otherwise
    """
    debug_print("\n[AUTH] Authenticating with atCloud365...")
    debug_print(f"[AUTH] Device SN: {DEVICE_SN}")
    debug_print(f"[AUTH] Sensor IDs: {SENSOR_IDS}")
    
    try:
        # Prepare authentication payload
        payload = {
            "sn": DEVICE_SN,
            "client_secret_key": CLIENT_SECRET_KEY,
            "sensorIds": SENSOR_IDS
        }
        
        debug_print(f"[AUTH] URL: {DEVICE_AUTH_URI}")
        debug_print(f"[AUTH] Payload: {json.dumps(payload, indent=2)}")
        
        # Send POST request
        response = requests.post(
            DEVICE_AUTH_URI,
            json=payload,
            headers={"Content-Type": "application/json"},
            timeout=30
        )
        
        debug_print(f"[AUTH] Response Status: {response.status_code}")
        
        if response.status_code == 200:
            data = response.json()
            debug_print(f"[AUTH] Response: {json.dumps(data, indent=2)}")
            
            if "token" in data:
                token = data["token"]
                debug_print("[AUTH] ✅ Authentication successful!")
                return token
            else:
                print("[AUTH] ❌ No token in response")
                return None
        else:
            print(f"[AUTH] ❌ HTTP Error: {response.status_code}")
            print(f"[AUTH] Response: {response.text}")
            return None
            
    except requests.exceptions.RequestException as e:
        print(f"[AUTH] ❌ Request failed: {e}")
        return None
    except Exception as e:
        print(f"[AUTH] ❌ Unexpected error: {e}")
        return None

# ==================================================
# Socket.IO Event Handlers
# ==================================================
@sio.event
def connect():
    """Handle Socket.IO connection."""
    global bootup_ready, last_status_report
    
    debug_print(f"\n[SOCKET] ✅ Connected! Socket ID: {sio.sid}")
    
    # Send bootup status
    if not bootup_ready:
        bootup_ready = True
        emit_dev_status("Bootup & Ready")
    else:
        emit_dev_status("Reconnected")
    
    # Send initial data
    emit_dev_data()
    last_status_report = get_current_time()

@sio.event
def disconnect():
    """Handle Socket.IO disconnection."""
    print("\n[SOCKET] ❌ Disconnected!")

@sio.event
def connect_error(data):
    """Handle Socket.IO connection error."""
    print(f"\n[SOCKET] ❌ Connection error: {data}")

@sio.event
def connected(data):
    """Handle custom 'connected' event from server."""
    debug_print(f"[SOCKET] Server confirmed connection: {data}")

@sio.on('app-cmd')
def handle_app_cmd(data):
    """
    Handle control commands from platform.
    
    Args:
        data: Command data dict
    """
    debug_print(f"\n[CMD] Received app-cmd: {json.dumps(data, indent=2)}")
    
    try:
        if "operation" not in data:
            debug_print("[CMD] ⚠️ No operation field in command")
            return
        
        operation = data["operation"]
        
        # Extract command parameters
        custom_cmd = operation.get("customCmd", "")
        field_index = operation.get("fieldIndex", -1)
        field_value = operation.get("fieldValue", -1)
        
        debug_print(f"[CMD] Parsed - cmd: {custom_cmd}, index: {field_index}, value: {field_value}")
        
        # Process command
        if custom_cmd == "output":
            if 0 <= field_index < len(output_pins) and field_value >= 0:
                set_output_state(field_index, field_value > 0)
        elif custom_cmd == "output-all":
            if field_value >= 0:
                set_all_outputs(field_value > 0)
        elif custom_cmd == "blinkLed":
            if 0 <= field_index < len(output_pins):
                count = field_value if field_value > 0 else 5
                set_output_blink(field_index, count)
        elif custom_cmd == "sync":
            # Force data sync
            emit_dev_data()
        elif custom_cmd == "reboot":
            print("[CMD] Reboot command received - exiting...")
            emit_dev_status("Rebooting")
            time.sleep(1)
            sys.exit(0)
        elif not custom_cmd:
            # Simple index/value control
            if 0 <= field_index < len(output_pins) and field_value >= 0:
                set_output_state(field_index, field_value > 0)
        else:
            debug_print(f"[CMD] ⚠️ Unknown command: {custom_cmd}")
            
    except Exception as e:
        print(f"[CMD] ❌ Error processing command: {e}")

# ==================================================
# Output Control Functions
# ==================================================
def set_output_state(index: int, state: bool):
    """
    Set output pin state.
    
    Args:
        index: Pin index
        state: True for ON, False for OFF
    """
    global state_changed
    
    if 0 <= index < len(output_pins):
        output_pins[index].state = state
        output_pins[index].blink_count = 0  # Stop blinking
        state_changed = True
        
        status = "ON" if state else "OFF"
        print(f"[OUTPUT] Pin {index} ({output_pins[index].name}): {status}")

def set_all_outputs(state: bool):
    """
    Set all output pins to same state.
    
    Args:
        state: True for ON, False for OFF
    """
    for i in range(len(output_pins)):
        set_output_state(i, state)
    
    status = "ON" if state else "OFF"
    print(f"[OUTPUT] All pins set to: {status}")

def set_output_blink(index: int, count: int):
    """
    Start blinking an output pin.
    
    Args:
        index: Pin index
        count: Number of blinks
    """
    if 0 <= index < len(output_pins):
        output_pins[index].blink_count = count * 2  # *2 for on+off cycles
        print(f"[OUTPUT] Pin {index} ({output_pins[index].name}) blink started (count: {count})")

def handle_blink_logic():
    """Handle blink logic for all outputs."""
    global last_blink_toggle, state_changed
    
    current_time = get_current_time()
    
    if current_time - last_blink_toggle < BLINK_INTERVAL:
        return
    
    last_blink_toggle = current_time
    
    for pin in output_pins:
        if pin.blink_count > 0:
            pin.state = not pin.state
            pin.blink_count -= 1
            state_changed = True
            
            if pin.blink_count == 0:
                pin.state = False  # Ensure OFF when done
                print(f"[OUTPUT] Pin {pin.index} blink completed")

# ==================================================
# Data Emission Functions
# ==================================================
def emit_dev_data():
    """Emit output states to platform."""
    global state_changed
    
    if not sio.connected:
        debug_print("[DATA] ⚠️ Socket not connected, skipping data upload")
        return
    
    try:
        # Prepare data payload
        content = [1 if pin.state else 0 for pin in output_pins]
        payload = {
            "content": content
        }
        
        # Emit data event
        sio.emit("dev-data", payload)
        debug_print(f"[DATA] 📤 Emitted: {json.dumps(payload)}")
        
        state_changed = False
        
    except Exception as e:
        print(f"[DATA] ❌ Error emitting data: {e}")

def emit_dev_status(status: str):
    """
    Emit device status to platform.
    
    Args:
        status: Status message string
    """
    if not sio.connected:
        debug_print("[STATUS] ⚠️ Socket not connected, skipping status upload")
        return
    
    try:
        sio.emit("dev-status", status)
        debug_print(f"[STATUS] 📤 Emitted: {status}")
    except Exception as e:
        print(f"[STATUS] ❌ Error emitting status: {e}")

# ==================================================
# Status Display
# ==================================================
def display_status():
    """Display current output states."""
    print("\n" + "="*50)
    print("CURRENT OUTPUT STATUS")
    print("="*50)
    for pin in output_pins:
        status = "🟢 ON" if pin.state else "⚫ OFF"
        blink = f" (Blinking: {pin.blink_count // 2})" if pin.blink_count > 0 else ""
        print(f"  Pin {pin.index} ({pin.name}): {status}{blink}")
    print("="*50 + "\n")

# ==================================================
# Main Loop
# ==================================================
def main_loop():
    """Main event loop for periodic tasks."""
    global last_status_report, state_changed, running
    
    # Display initial status
    display_status()
    
    status_display_counter = 0
    
    while running:
        try:
            current_time = get_current_time()
            
            # Handle blink logic
            handle_blink_logic()
            
            # Check if state changed
            if state_changed and sio.connected:
                emit_dev_data()
            
            # Periodic status report
            if sio.connected and (current_time - last_status_report >= STATUS_REPORT_INTERVAL):
                emit_dev_status("Status OK")
                last_status_report = current_time
            
            # Display status every 10 seconds
            status_display_counter += 1
            if status_display_counter >= 100:  # 100 * 0.1s = 10s
                display_status()
                status_display_counter = 0
            
            # Small sleep to prevent busy loop
            time.sleep(0.1)
            
        except KeyboardInterrupt:
            print("\n[MAIN] Interrupted by user")
            running = False
            break
        except Exception as e:
            print(f"[MAIN] ❌ Error in main loop: {e}")
            time.sleep(1)

# ==================================================
# Signal Handler
# ==================================================
def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully."""
    global running
    print("\n[SIGNAL] Received interrupt signal, shutting down...")
    running = False
    
    if sio.connected:
        emit_dev_status("Shutting down")
        time.sleep(0.5)
        sio.disconnect()
    
    sys.exit(0)

# ==================================================
# Main Entry Point
# ==================================================
def main():
    """Main entry point."""
    global auth_token, running
    
    print("\n" + "="*50)
    print("atCloud365 Output Device Example")
    print("Python SDK Version")
    print("="*50 + "\n")
    
    # Register signal handler
    signal.signal(signal.SIGINT, signal_handler)
    
    # Step 1: Authenticate
    auth_token = authenticate_device()
    if not auth_token:
        print("\n❌ Authentication failed! Exiting...")
        sys.exit(1)
    
    # Step 2: Connect to Socket.IO
    try:
        # Build socket URL with query string (python-socketio connect doesn't accept 'query' kw)
        query_params = {
            "sn": DEVICE_SN,
            "clientType": "device",
            "sensorIds": json.dumps(SENSOR_IDS),
            "clientVersion": "V4"
        }
        socket_url = f"{SERVER_URL}?{urlencode(query_params)}"
        
        debug_print(f"\n[SOCKET] Connecting to {socket_url}")
        debug_print(f"[SOCKET] Path: {API_PATH}")
        debug_print(f"[SOCKET] Query: sn={DEVICE_SN}, sensorIds={json.dumps(SENSOR_IDS)}")
        
        sio.connect(
            socket_url,
            socketio_path=API_PATH,
            auth={"token": auth_token},
            transports=["websocket", "polling"],
            headers={},
            wait_timeout=10,
            namespaces=["/"]
        )
        
        debug_print("[SOCKET] Connection initiated...")
        
    except Exception as e:
        print(f"\n[SOCKET] ❌ Connection failed: {e}")
        sys.exit(1)
    
    # Step 3: Run main loop
    try:
        print("\n💡 Waiting for commands from platform...")
        print("   Press Ctrl+C to exit\n")
        main_loop()
    except KeyboardInterrupt:
        print("\n[MAIN] Interrupted by user")
    finally:
        running = False
        if sio.connected:
            emit_dev_status("Shutting down")
            time.sleep(0.5)
            sio.disconnect()
        print("\n[MAIN] Bye! 👋\n")

if __name__ == "__main__":
    main()
