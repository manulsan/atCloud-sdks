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
#define DEVICE_SN "03EB023C002601000000FC"
#define CLIENT_SECRET_KEY "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"

// ==================================================
// atCloud365 Server Configuration
// ==================================================
#define SERVER_URL "https://atcloud365.com"
#define SERVER_PORT 443
#define API_PATH "/api/dev/io/"

// ==================================================
// Sensor Configuration
// ==================================================
// Base sensor ID and sensor count used in the authentication payload.
// Modify these values to match your hardware configuration.
#define BASE_SENSOR_ID 0x0f1234
#define SENSOR_COUNT 3

// ==================================================
// Timeout Settings
// ==================================================
#define HTTP_TIMEOUT 30000     // 30 seconds
#define SOCKETIO_TIMEOUT 60000 // 60 seconds

// ==================================================
// GPIO Pin Configuration (ESP32)
// ==================================================
// Input pins for sensors (pulled up internally)
#define GPIO_INPUT_1 32
#define GPIO_INPUT_2 33
#define GPIO_INPUT_3 25

// ==================================================
// LCD Configuration (ST7789P3 240x320 — module: ESP32-32E-7789)
// Default pins for the module (change if your wiring differs)
// ==================================================
#ifdef HAS_LCD_240x320
// SPI pins (module default)
#define LCD_SCK_PIN 14
#define LCD_MOSI_PIN 13
#define LCD_MISO_PIN 12
#define LCD_CS_PIN 15
#define LCD_DC_PIN 2
#define LCD_RST_PIN 22
#define LCD_BL_PIN 21
#endif

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
