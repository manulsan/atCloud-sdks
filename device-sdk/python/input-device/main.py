#!/usr/bin/env python3
"""
atCloud365 Input Device - Python SDK Example

This example demonstrates how to:
1. Authenticate with atCloud365 platform via HTTPS POST
2. Connect to Socket.IO for real-time communication
3. Read sensor data (simulated with keyboard input)
4. Send data to platform via "dev-data" event
5. Send status updates via "dev-status" event

Author: atCloud365
Date: 2026-02-12
"""

import sys
import time
import json
import random
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
        DATA_UPLOAD_INTERVAL,
        STATUS_REPORT_INTERVAL,
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
sensor_values: List[int] = [0] * len(SENSOR_IDS)
last_data_upload: float = 0
last_status_report: float = 0
state_changed: bool = False
running: bool = True

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

def get_random(min_val: int, max_val: int) -> int:
    """Return a random integer between min_val and max_val inclusive."""
    return random.randint(min_val, max_val)

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
    global bootup_ready, last_data_upload, last_status_report
    
    debug_print(f"\n[SOCKET] ✅ Connected! Socket ID: {sio.sid}")
    
    # Send bootup status
    if not bootup_ready:
        bootup_ready = True
        emit_dev_status("Bootup & Ready")
    else:
        emit_dev_status("Reconnected")
    
    # Send initial data
    emit_dev_data()
    last_data_upload = get_current_time()
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
        if custom_cmd == "sync":
            # Force data sync
            emit_dev_data()
        elif custom_cmd == "reboot":
            print("[CMD] Reboot command received - exiting...")
            emit_dev_status("Rebooting")
            time.sleep(1)
            sys.exit(0)
        else:
            debug_print(f"[CMD] ⚠️ Unknown command: {custom_cmd}")
            
    except Exception as e:
        print(f"[CMD] ❌ Error processing command: {e}")

# ==================================================
# Data Emission Functions
# ==================================================
def emit_dev_data():
    """Emit sensor data to platform."""
    global last_data_upload
    
    if not sio.connected:
        debug_print("[DATA] ⚠️ Socket not connected, skipping data upload")
        return
    
    try:
        # Simulate new sensor values before sending (index-based ranges)
        for i in range(len(sensor_values)):
            sensor_values[i] = get_random(i * 10, i * 10 + 10)
        
        # Prepare data payload
        payload = {
            "content": sensor_values
        }
        
        # Emit data event
        sio.emit("dev-data", payload)
        debug_print(f"[DATA] 📤 Emitted: {json.dumps(payload)}")
        
        last_data_upload = get_current_time()
        
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
# Sensor Simulation (Keyboard Input)
# ==================================================
def simulate_sensor_input():
    """
    Simulate sensor input using keyboard.
    User can enter sensor values in format: index,value
    Example: 0,100  (set sensor 0 to value 100)
    """
    global sensor_values, state_changed, running
    
    print("\n" + "="*50)
    print("SENSOR INPUT SIMULATION")
    print("="*50)
    print("Enter sensor values in format: index,value")
    print(f"Available indices: 0-{len(SENSOR_IDS)-1}")
    print("Example: 0,100  (set sensor 0 to value 100)")
    print("Type 'quit' to exit")
    print("="*50 + "\n")
    
    while running:
        try:
            user_input = input("Enter sensor value (index,value): ").strip()
            
            if user_input.lower() in ['quit', 'exit', 'q']:
                print("\n[INPUT] Exiting...")
                running = False
                break
            
            if not user_input:
                continue
            
            # Parse input
            parts = user_input.split(',')
            if len(parts) != 2:
                print("[INPUT] ⚠️ Invalid format. Use: index,value")
                continue
            
            index = int(parts[0].strip())
            value = int(parts[1].strip())
            
            if index < 0 or index >= len(SENSOR_IDS):
                print(f"[INPUT] ⚠️ Invalid index. Use 0-{len(SENSOR_IDS)-1}")
                continue
            
            # Update sensor value
            sensor_values[index] = value
            state_changed = True
            
            print(f"[INPUT] ✅ Sensor {index} set to {value}")
            print(f"[INPUT] Current values: {sensor_values}")
            
        except ValueError:
            print("[INPUT] ⚠️ Invalid input. Use numeric values only.")
        except KeyboardInterrupt:
            print("\n[INPUT] Interrupted by user")
            running = False
            break
        except Exception as e:
            print(f"[INPUT] ❌ Error: {e}")

# ==================================================
# Main Loop
# ==================================================
def main_loop():
    """Main event loop for periodic tasks."""
    global last_data_upload, last_status_report, state_changed, running
    
    while running:
        try:
            current_time = get_current_time()
            
            # Check if state changed
            if state_changed and sio.connected:
                emit_dev_data()
                state_changed = False
            
            # Periodic data upload
            if sio.connected and (current_time - last_data_upload >= DATA_UPLOAD_INTERVAL):
                emit_dev_data()
            
            # Periodic status report
            if sio.connected and (current_time - last_status_report >= STATUS_REPORT_INTERVAL):
                emit_dev_status("Status OK")
                last_status_report = current_time
            
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
    print("atCloud365 Input Device Example")
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
    
    # Step 3: Start input simulation thread
    input_thread = threading.Thread(target=simulate_sensor_input, daemon=True)
    input_thread.start()
    
    # Step 4: Run main loop
    try:
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
