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
// Output pins for actuators/relays/LEDs
#define GPIO_OUTPUT_1 19
#define GPIO_OUTPUT_2 21
#define GPIO_OUTPUT_3 22

// ==================================================
// LCD Configuration (ILI9341 240x320)
// ==================================================
#ifdef HAS_LCD_240x320
// SPI pins (change if needed)
#define LCD_SCK_PIN 18
#define LCD_MOSI_PIN 23
#define LCD_MISO_PIN 19
#define LCD_CS_PIN 5
#define LCD_DC_PIN 21
#define LCD_RST_PIN 22
#define LCD_BL_PIN 4
#endif

// ==================================================
// Timing Configuration
// ==================================================
#define STATUS_REPORT_INTERVAL 60000 // Report status every 60s
#define BLINK_INTERVAL 500           // Blink toggle interval (500ms)

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
