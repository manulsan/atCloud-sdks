#ifndef CONFIG_H
#define CONFIG_H

// ==================================================
// WiFi Configuration
// ==================================================
#define WIFI_SSID "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

// ==================================================
// atCloud365 Authentication
// NOTE: This is for testing purpose only
// For production, use credentials from atCloud365 platform
// ==================================================
#define DEVICE_SN "03EB023C002601000000FF"
#define CLIENT_SECRET_KEY "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"

// ==================================================
// atCloud365 Server Configuration
// ==================================================
#define SERVER_URL "https://atcloud365.com"
#define SERVER_PORT 443
#define API_PATH "/api/dev/io/"

// ==================================================
// Timeout Settings
// ==================================================
#define HTTP_TIMEOUT 30000     // 30 seconds
#define SOCKETIO_TIMEOUT 60000 // 60 seconds

// ==================================================
// GPIO Pin Configuration (ESP32)
// ==================================================
// Input pins for sensors (pulled up internally)
#define GPIO_INPUT_1 19
#define GPIO_INPUT_2 21
#define GPIO_INPUT_3 22

// ==================================================
// Timing Configuration
// ==================================================
#define GPIO_SCAN_INTERVAL 100   // Scan GPIO every 100ms
#define DATA_SEND_INTERVAL 60000 // Send periodic update every 60s

// ==================================================
// Debug Configuration
// ==================================================
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
