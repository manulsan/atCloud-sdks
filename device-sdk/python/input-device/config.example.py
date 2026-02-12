"""
atCloud365 Python SDK Configuration

Copy this file to config.py and update with your settings.
NEVER commit config.py to version control!
"""

# ==================================================
# Device Authentication
# NOTE: This is for testing purpose only
# For production, use credentials from atCloud365 platform
# ==================================================
DEVICE_SN = "03EB023C002601000000FE"
CLIENT_SECRET_KEY = "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"

# ==================================================
# Server Configuration
# ==================================================
SERVER_URL = "https://atcloud365.com"
SERVER_PORT = 443
API_PATH = "/api/dev/io/"

# Device authentication endpoint
DEVICE_AUTH_URI = "https://atcloud365.com/api/v3/devices/auth"

# ==================================================
# Sensor Configuration
# ==================================================
# Sensor IDs (hex values)
# Example: [0x0f1234, 0x0f1235, 0x0f1236]
SENSOR_IDS = [0x0f1234, 0x0f1235, 0x0f1236]

# ==================================================
# Timing Configuration
# ==================================================
# Data upload interval (seconds)
DATA_UPLOAD_INTERVAL = 10

# Status report interval (seconds)
STATUS_REPORT_INTERVAL = 60

# ==================================================
# Debug Configuration
# ==================================================
DEBUG_ENABLED = True
