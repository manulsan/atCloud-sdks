/**
 * @file main.cpp
 * @brief atCloud365 Input Device - Sensor Data Collection Example
 *
 * This example demonstrates how to:
 * 1. Connect to WiFi
 * 2. Authenticate with atCloud365 platform via HTTPS
 * 3. Connect to Socket.IO for real-time communication
 * 4. Read GPIO inputs (sensors)
 * 5. Send data when state changes or periodically
 *
 * @author atCloud365
 * @date 2026-02-12
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"
#include "main.h"
#ifdef HAS_LCD_240x320
#include "lcd.h"
#include "lcd_app.h"
#endif

SocketIOClient socketIo;

#ifdef HAS_LCD_240x320
static void collectUiState(UiSnapshot &ui)
{
    ui.wifiConnected = WiFi.status() == WL_CONNECTED;
    ui.wifiRssi = ui.wifiConnected ? WiFi.RSSI() : 0;
    ui.socketConnected = socketConnected;

    for (int i = 0; i < SENSOR_COUNT; i++)
        ui.sensors[i] = (gpioInputs[i].state == LOW);

    static bool ntpInit = false;
    if (!ntpInit)
    {
        const long gmtOffset = 9 * 3600; // KST (UTC+9)
        const int daylightOffset = 0;
        configTime(gmtOffset, daylightOffset, "pool.ntp.org", "time.nist.gov");
        ntpInit = true;
    }

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100))
    {
        char dateBuf[16];
        char timeBuf[12];
        strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &timeinfo);
        strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &timeinfo);
        ui.dateText = String(dateBuf);
        ui.timeText = String(timeBuf);
    }
    else
    {
        ui.dateText = "--";
        ui.timeText = "--:--:--";
    }
    ui.ip = WiFi.localIP();
}
#endif

// ==================================================
// Setup
// ==================================================
void setup()
{
    Serial.begin(115200);
    delay(100);

    DEBUG_PRINTLN("\n\n========================================");
    DEBUG_PRINTLN("atCloud365 Input Device Example");
    DEBUG_PRINTLN("========================================\n");

    // Initialize GPIO pins as inputs with pullup
    DEBUG_PRINTLN("[GPIO] Initializing input pins...");
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        pinMode(gpioInputs[i].pin, INPUT_PULLUP);
        gpioInputs[i].state = digitalRead(gpioInputs[i].pin);
        gpioInputs[i].previousState = gpioInputs[i].state;
        DEBUG_PRINTF("  GPIO %d: %d\n", gpioInputs[i].pin, gpioInputs[i].state);
    }
    setupWiFi();

    // GET Authentication Token
    if (authenticateDevice())
    {
        DEBUG_PRINTLN("[AUTH] Authentication successful!");

        // Connect to Socket.IO (use SocketIOClient)
        socketIo.setPacketCallback(handleSocketIOPacket);
        socketIo.setConnectCallback([]()
                                    { DEBUG_PRINTLN("[SOCKET] SocketIO connected (callback)"); });
        socketIo.setDisconnectCallback([]()
                                       {
            DEBUG_PRINTLN("[SOCKET] SocketIO disconnected (callback)");
            socketConnected = false; });
        socketIo.begin(authToken);

#ifdef HAS_LCD_240x320
        DEBUG_PRINTLN("[LCD] Initializing and running visual test...");
        LCD::begin();
        LCD::setBacklight(255);
        LCD::setRotation(2); // 180-degree rotation to match panel mounting
        LCD::fillScreen(0x0000);

        // Draw the operational dashboard (WiFi, Socket, Sensors, Clock)
        UiSnapshot uiState;
        collectUiState(uiState);
        lcdUiRender(uiState);
#endif
    }
    else
    {
        DEBUG_PRINTLN("[AUTH] Authentication failed! Rebooting in 10 seconds...");
        delay(10000);
        ESP.restart();
    }
}

// ==================================================
// Main Loop
// ==================================================
void loop()
{
    socketIo.loop();

    // Scan GPIO inputs periodically
    if (millis() - lastGpioScan >= GPIO_SCAN_INTERVAL)
    {
        scanGpioInputs(); // if (scanGpioInputs())     dataUpdateRequired = true;
        lastGpioScan = millis();
    }
    if (socketConnected)
    {
        // Send periodic update even if no change
        if ((millis() - lastDataSend >= DATA_SEND_INTERVAL))
            dataUpdateRequired = true;

        // Send data when required
        if (dataUpdateRequired)
        {
            emitDevData();
            dataUpdateRequired = false;
            lastDataSend = millis();
        }
    }

    //----------------------------------------------------------
    // LCD UI Update (only if we have a display and data changed)
    //----------------------------------------------------------
#ifdef HAS_LCD_240x320
    static unsigned long lastUiRefresh = 0;

    if ((millis() - lastUiRefresh > 1000) || dataUpdateRequired)
    {
        UiSnapshot uiState;
        collectUiState(uiState);
        lcdUiRender(uiState);
        lastUiRefresh = millis();
    }
#endif
}

// ==================================================
// WiFi Setup
// ==================================================
void setupWiFi()
{
    DEBUG_PRINT("[WiFi] Connecting to ");
    DEBUG_PRINTLN(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30)
    {
        delay(500);
        DEBUG_PRINT(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        DEBUG_PRINTLN("\n[WiFi] Connected!");
        DEBUG_PRINT("[WiFi] IP Address: ");
        DEBUG_PRINTLN(WiFi.localIP());
        DEBUG_PRINT("[WiFi] RSSI: ");
        DEBUG_PRINT(WiFi.RSSI());
        DEBUG_PRINTLN(" dBm");
    }
    else
    {
        DEBUG_PRINTLN("\n[WiFi] Connection failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
}

// ==================================================
// HTTPS Authentication
// ==================================================
bool authenticateDevice()
{
    DEBUG_PRINTLN("\n[AUTH] Authenticating with atCloud365...");
    DEBUG_PRINTF("[AUTH] Device SN: %s\n", DEVICE_SN);

    HTTPClient https;
    https.setTimeout(HTTP_TIMEOUT);

    // Build authentication URL (use HTTP POST with JSON payload like Node.js example)
    String authUrl = String(SERVER_URL) + "/api/v3/devices/auth";

    // Build JSON payload (include sensorIds like Node.js example)
    JsonDocument doc;
    doc["sn"] = String(DEVICE_SN);
    doc["client_secret_key"] = String(CLIENT_SECRET_KEY);

    // sensorIds: generated from base ID and sensor count (configurable via config.h)
    JsonArray sensorIds = doc["sensorIds"].to<JsonArray>();
    for (size_t i = 0; i < SENSOR_COUNT; i++)
        sensorIds.add((uint32_t)(BASE_SENSOR_ID + i));

    String postPayload;
    serializeJson(doc, postPayload);

    DEBUG_PRINTF("[AUTH] URL: %s\n", authUrl.c_str());
    DEBUG_PRINTF("[AUTH] Payload: %s\n", postPayload.c_str());

    https.begin(authUrl);
    https.addHeader("Content-Type", "application/json");

    int httpCode = https.POST(postPayload);

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = https.getString();
        DEBUG_PRINTF("[AUTH] Response: %s\n", payload.c_str());

        // Parse JSON response
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error && doc["token"].is<const char *>())
        {
            authToken = doc["token"].as<String>();
            DEBUG_PRINTLN("[AUTH] Token received successfully");
            https.end();
            return true;
        }
        else
        {
            DEBUG_PRINTLN("[AUTH] Invalid response format");
        }
    }
    else
    {
        DEBUG_PRINTF("[AUTH] HTTP Error: %d\n", httpCode);
    }

    https.end();
    return false;
}

// ==================================================
// Socket.IO Connection
// ==================================================
void connectSocketIO()
{
    // connect using the SocketIOClient wrapper (authToken must be set)
    socketIo.begin(authToken);
    socketIo.setReconnectInterval(5000);
    DEBUG_PRINTLN("[SOCKET] Connection initiated via SocketIOClient");
}

// ==================================================
// WebSocket Event Handler
// ==================================================
// WebSocket event handler moved into SocketIOClient class; main registers callbacks if needed.

// ==================================================
// Socket.IO Packet Handler
// ==================================================
void handleSocketIOPacket(const char *packet, size_t length)
{
    if (length == 0)
        return;

    char packetType = packet[0];

    switch (packetType)
    {
    case '0': // Connection info (open)
    {
        DEBUG_PRINTLN("[SOCKET] Connection info received");
        const char *jsonStr = packet + 1;
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonStr);

        if (!error)
        {
            if (doc["sid"].is<const char *>())
            {
                socketSid = doc["sid"].as<String>();
                DEBUG_PRINTF("[SOCKET] SID: %s\n", socketSid.c_str());
            }

            // Send connect acknowledgment with auth payload (Socket.IO expects token here)
            String connectPayload = String("{\"token\":\"") + authToken + "\"}";
            sendPacket("40", connectPayload);
            socketConnected = true;

            // Send bootup status
            if (!bootupReady)
            {
                bootupReady = true;
                emitDevStatus("Bootup & Ready");
            }
            else
            {
                // emitDevStatus("Reconnected");
            }
        }
    }
    break;

    case '2': // Ping
        DEBUG_PRINTLN("[SOCKET] Ping received, sending pong");
        sendPacket("3");
        break;

    case '3': // Pong
        DEBUG_PRINTLN("[SOCKET] Pong received");
        break;

    case '4': // Message
        if (length > 1)
        {
            char messageType = packet[1];

            if (messageType == '0')
            {
                DEBUG_PRINTLN("[SOCKET] Connection acknowledged (40)");
            }
            else if (messageType == '2')
            {
                // Event message: 42["event", data]
                const char *eventData = packet + 2;
                DEBUG_PRINTF("[SOCKET] Event received: %s\n", eventData);

                // Parse event (for input device, we mainly listen to 'connected' confirmation)
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, eventData);

                if (!error && doc.is<JsonArray>())
                {
                    JsonArray arr = doc.as<JsonArray>();
                    if (arr.size() > 0)
                    {
                        String eventName = arr[0].as<String>();
                        DEBUG_PRINTF("[SOCKET] Event name: %s\n", eventName.c_str());

                        if (eventName == "connected")
                        {
                            DEBUG_PRINTLN("[SOCKET] Server confirmed connection!");
                        }
                        //  ["app-cmd",{"operation":{"customCmd":"clear-call-bell","fieldIndex":2,"fieldValue":0}}]
                        else if (eventName == "app-cmd" && arr.size() > 1)
                        {
                            JsonObject eventPayload = arr[1].as<JsonObject>();
                            if (eventPayload["operation"].is<JsonObject>())
                            {
                                JsonObject operation = eventPayload["operation"].as<JsonObject>();
                                if (operation["customCmd"].is<const char *>())
                                {
                                    String customCmd = operation["customCmd"].as<String>();
                                    DEBUG_PRINTF("[SOCKET] Custom Command: %s\n", customCmd.c_str());
                                    if (customCmd == "clear-call-bell" || customCmd == "output")
                                    {
                                        uint8_t fieldIndex = operation["fieldIndex"] | 0;
                                        uint8_t fieldValue = operation["fieldValue"] | 0;
                                        DEBUG_PRINTF("[SOCKET] clear-call-bell - Index: %d, Value: %d\n", fieldIndex, fieldValue);
                                        if (fieldIndex < SENSOR_COUNT && fieldValue >= 0)
                                            gpioInputs[fieldIndex]
                                                .state = fieldValue == 1 ? LOW : HIGH;
                                        gpioInputs[fieldIndex].previousState = gpioInputs[fieldIndex].state;
                                        dataUpdateRequired = true;
                                    }
                                    // Handle custom commands as needed
                                }
                            }
                        }
                    }
                }
            }
        }
        break;

    default:
        DEBUG_PRINTF("[SOCKET] Unknown packet type: %c\n", packetType);
        break;
    }
}

// ==================================================
// Send Socket.IO Packet (forward to socketIo)
// ==================================================
void sendPacket(const char *type, const String &data)
{
    socketIo.sendPacket(type, data);
    DEBUG_PRINTF("[SOCKET] Sent: %s%s\n", type, data.c_str());
}

// ==================================================
// Emit Dev-Data Event
// ==================================================
void emitDevData()
{
    JsonDocument doc;
    JsonArray content = doc["content"].to<JsonArray>();

    // Add GPIO states (inverted because INPUT_PULLUP: LOW=pressed/active, HIGH=released/inactive)
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        content.add(gpioInputs[i].state == LOW ? 1 : 0);
    }

    String jsonData;
    serializeJson(doc, jsonData);

    // Socket.IO event format: 42["event-name", data]
    String packet = "42[\"dev-data\"," + jsonData + "]";

    // send via SocketIO wrapper
    sendPacket(packet.c_str());
    DEBUG_PRINTF("[DATA] Emitted: %s\n", packet.c_str());
}

// ==================================================
// Emit Dev-Status Event
// ==================================================
void emitDevStatus(const String &status)
{
    // Socket.IO event format: 42["event-name", "data"]
    String packet = "42[\"dev-status\",\"" + status + "\"]";

    sendPacket(packet.c_str());
    DEBUG_PRINTF("[STATUS] Emitted: %s\n", packet.c_str());
}

// ==================================================
// Scan GPIO Inputs or Simulate Changes
// ==================================================
bool scanGpioInputs()
{
    bool changed = false;

    for (int i = 0; i < SENSOR_COUNT; i++)
    {
#ifdef USE_SIMULATED_GPIO_VALUES
        // uint32_t randomValue = esp_random();
        gpioInputs[i].state = esp_random() % 2 == 0 ? LOW : HIGH; // Randomly toggle state
                                                                  // DEBUG_PRINTF("[randomValue] value: %d\n", randomValue);
#else
        gpioInputs[i].state = digitalRead(gpioInputs[i].pin);
#endif
        if (gpioInputs[i].state != gpioInputs[i].previousState)
        {
            DEBUG_PRINTF("[GPIO] Pin %d changed: %d -> %d\n",
                         gpioInputs[i].pin,
                         gpioInputs[i].previousState,
                         gpioInputs[i].state);
            gpioInputs[i].previousState = gpioInputs[i].state;
            changed = true;
        }
    }
    // return changed;
#ifdef USE_SIMULATED_GPIO_VALUES
    return false;
#endif
    return changed;
}
