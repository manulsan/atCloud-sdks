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
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "main.h"
#ifdef HAS_LCD_240x320
#include "lcd.h"
#endif

// Globals and function implementations remain in this file; declarations moved to `include/main.h`.

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

    // Connect to WiFi
    setupWiFi();

    // GET Authentication Token
    if (authenticateDevice())
    {
        DEBUG_PRINTLN("[AUTH] Authentication successful!");

        // Connect to Socket.IO
        connectSocketIO();

#ifdef HAS_LCD_240x320
        DEBUG_PRINTLN("[LCD] Initializing...");
        LCD::begin();
        LCD::fillScreen(0x001F);                 // Blue
        LCD::drawRect(10, 10, 220, 100, 0xF800); // Red rectangle
        LCD::setBacklight(255);

        // Draw "Hello" centered-ish inside the red rectangle
        // (uses a small block-font renderer implemented below)
        lcdDrawText("Hello", 60, 40, 0xFFFF, 2);

        DEBUG_PRINTLN("[LCD] Init done.");
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
    // Handle WebSocket events
    webSocket.loop();

#ifndef USE_SIMULATED_GPIO_VALUES
    // Scan GPIO inputs periodically
    if (millis() - lastGpioScan >= GPIO_SCAN_INTERVAL)
    {
        lastGpioScan = millis();
        if (scanGpioInputs())
            dataUpdateRequired = true;
    }
#endif

    if (socketConnected)
    {
        // Send periodic update even if no change
        if ((millis() - lastDataSend >= DATA_SEND_INTERVAL))
        {
#ifdef USE_SIMULATED_GPIO_VALUES
            // Simulate input value changes for testing
            for (int i = 0; i < SENSOR_COUNT; i++)
            {
                gpioInputs[i].state = millis() % 2 == 1 ? HIGH : LOW;
                delay(i);
            }
#endif
            dataUpdateRequired = true;
        }

        // Send data when required
        if (dataUpdateRequired)
        {
            emitDevData();
            dataUpdateRequired = false;
            lastDataSend = millis();
        }

        // Simple heartbeat check
        if ((millis() - lastPingTime > 70000))
        {
            DEBUG_PRINTLN("[SOCKET] Connection timeout, reconnecting...");
            socketConnected = false;
            connectSocketIO();
        }
    }
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
    DynamicJsonDocument doc(256);
    doc["sn"] = String(DEVICE_SN);
    doc["client_secret_key"] = String(CLIENT_SECRET_KEY);

    // sensorIds: generated from base ID and sensor count (configurable via config.h)
    const uint32_t baseSensorId = 0x0f1234;
    JsonArray sensorIds = doc.createNestedArray("sensorIds");
    for (size_t i = 0; i < SENSOR_COUNT; i++)
        sensorIds.add((uint32_t)(baseSensorId + i));

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

        if (!error && doc.containsKey("token"))
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
    DEBUG_PRINTLN("\n[SOCKET] Connecting to Socket.IO...");

    // Extract domain from SERVER_URL
    String domain = String(SERVER_URL);
    domain.replace("https://", "");
    domain.replace("http://", "");

    // Build Socket.IO path with query parameters
    String socketPath = String(API_PATH) +
                        "?sn=" + String(DEVICE_SN) +
                        "&token=" + authToken +
                        "&EIO=4&transport=websocket";

    DEBUG_PRINTF("[SOCKET] Domain: %s\n", domain.c_str());
    DEBUG_PRINTF("[SOCKET] Port: %d\n", SERVER_PORT);
    DEBUG_PRINTF("[SOCKET] Path: %s\n", socketPath.c_str());

    // Connect via SSL
    webSocket.beginSSL(domain.c_str(), SERVER_PORT, socketPath.c_str());
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);

    DEBUG_PRINTLN("[SOCKET] Connection initiated...");
}

// ==================================================
// WebSocket Event Handler
// ==================================================
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        DEBUG_PRINTLN("[SOCKET] Disconnected!");
        socketConnected = false;
        break;

    case WStype_CONNECTED:
        DEBUG_PRINTLN("[SOCKET] WebSocket connected");
        break;

    case WStype_TEXT:
    {
        char buffer[1024];
        size_t len = (length < sizeof(buffer) - 1) ? length : sizeof(buffer) - 1;
        memcpy(buffer, payload, len);
        buffer[len] = '\0';

        DEBUG_PRINTF("[SOCKET] Received: %s\n", buffer);
        handleSocketIOPacket(buffer, len);
        lastPingTime = millis();
    }
    break;

    case WStype_ERROR:
        DEBUG_PRINTLN("[SOCKET] Error occurred");
        break;

    default:
        break;
    }
}

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
            if (doc.containsKey("sid"))
            {
                socketSid = doc["sid"].as<String>();
                DEBUG_PRINTF("[SOCKET] SID: %s\n", socketSid.c_str());
            }

            // Send connect acknowledgment
            sendPacket("40");
            socketConnected = true;

            // Send bootup status
            if (!bootupReady)
            {
                bootupReady = true;
                emitDevStatus("Bootup & Ready");
            }
            else
            {
                emitDevStatus("Reconnected");
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
// Send Socket.IO Packet
// ==================================================
void sendPacket(const char *type, const String &data)
{
    String packet = String(type);
    if (data.length() > 0)
    {
        packet += data;
    }
    webSocket.sendTXT(packet);
    DEBUG_PRINTF("[SOCKET] Sent: %s\n", packet.c_str());
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

    webSocket.sendTXT(packet);
    DEBUG_PRINTF("[DATA] Emitted: %s\n", packet.c_str());
}

// ==================================================
// Emit Dev-Status Event
// ==================================================
void emitDevStatus(const String &status)
{
    // Socket.IO event format: 42["event-name", "data"]
    String packet = "42[\"dev-status\",\"" + status + "\"]";

    webSocket.sendTXT(packet);
    DEBUG_PRINTF("[STATUS] Emitted: %s\n", packet.c_str());
}

// --- Simple LCD drawing helpers (block font) -------------------------------
static void drawFilledRect(int x, int y, int w, int h, uint16_t color)
{
    for (int ix = 0; ix < w; ix++)
        for (int iy = 0; iy < h; iy++)
            LCD::drawPixel(x + ix, y + iy, color);
}

static void lcdDrawCharBlock(int x, int y, char c, uint16_t color, uint8_t scale)
{
    // Very small block-style glyphs for ASCII characters used in "Hello".
    switch (c)
    {
    case 'H':
        drawFilledRect(x, y, 3 * scale, 16 * scale, color);
        drawFilledRect(x + 8 * scale, y, 3 * scale, 16 * scale, color);
        drawFilledRect(x + 3 * scale, y + 6 * scale, 5 * scale, 3 * scale, color);
        break;

    case 'e':
        drawFilledRect(x, y + 2 * scale, 10 * scale, 3 * scale, color);            // top bar
        drawFilledRect(x, y + 2 * scale, 3 * scale, 12 * scale, color);            // left column
        drawFilledRect(x + 3 * scale, y + 8 * scale, 7 * scale, 3 * scale, color); // middle bar
        drawFilledRect(x, y + 14 * scale, 10 * scale, 3 * scale, color);           // bottom bar
        break;

    case 'l':
        drawFilledRect(x + 3 * scale, y, 3 * scale, 16 * scale, color);
        break;

    case 'o':
        LCD::drawRect(x, y, 10 * scale, 16 * scale, color);
        drawFilledRect(x + 2 * scale, y + 2 * scale, 6 * scale, 12 * scale, color);
        break;

    case ' ':
        // leave gap
        break;

    default:
        // fallback: small box for unknown characters
        LCD::drawRect(x, y, 6 * scale, 10 * scale, color);
        break;
    }
}

void lcdDrawText(const char *text, int x, int y, uint16_t color, uint8_t scale)
{
    int cursorX = x;
    for (const char *p = text; *p; ++p)
    {
        lcdDrawCharBlock(cursorX, y, *p, color, scale);
        cursorX += 12 * scale; // advance (letter width + spacing)
    }
}

// ==================================================
// Scan GPIO Inputs
// ==================================================
bool scanGpioInputs()
{
    bool changed = false;

    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        gpioInputs[i].state = digitalRead(gpioInputs[i].pin);
        if (gpioInputs[i].state != gpioInputs[i].previousState)
        {
            DEBUG_PRINTF("[GPIO] Pin %d changed: %d -> %d\n",
                         gpioInputs[i].pin,
                         gpioInputs[i].previousState,
                         gpioInputs[i].state);
            gpioInputs[i].previousState = gpioInputs[i].state;
            // changed = true;
        }
    }
    return changed;
}
