/**
 * @file main.cpp
 * @brief atCloud365 Output Device - Actuator Control Example
 *
 * This example demonstrates how to:
 * 1. Connect to WiFi
 * 2. Authenticate with atCloud365 platform via HTTPS
 * 3. Connect to Socket.IO for real-time communication
 * 4. Receive control commands from platform
 * 5. Control GPIO outputs (relays, LEDs, actuators)
 * 6. Send status updates back to platform
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

// Global definitions
WebSocketsClient webSocket;
bool socketConnected = false;
bool bootupReady = false;
String authToken = "";
String socketSid = "";

GpioOutput gpioOutputs[] = {
    {GPIO_OUTPUT_1, false, 0},
    {GPIO_OUTPUT_2, false, 0},
    {GPIO_OUTPUT_3, false, 0}};

unsigned long lastStatusReport = 0;
unsigned long lastBlinkToggle = 0;
unsigned long lastPingTime = 0;
bool stateChanged = true; // emit initial status

// Global variables and function implementations remain in this file.
// Declarations are provided by `include/main.h`.

// ==================================================
// Setup
// ==================================================
void setup()
{
    Serial.begin(115200);
    delay(100);

    DEBUG_PRINTLN("\n\n========================================");
    DEBUG_PRINTLN("atCloud365 Output Device Example");
    DEBUG_PRINTLN("========================================\n");

    // Initialize GPIO pins as outputs
    DEBUG_PRINTLN("[GPIO] Initializing output pins...");
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        pinMode(gpioOutputs[i].pin, OUTPUT);
        gpioOutputs[i].state = false;
        digitalWrite(gpioOutputs[i].pin, LOW);
        DEBUG_PRINTF("  GPIO %d: OFF\n", gpioOutputs[i].pin);
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
        LCD::fillScreen(0x07E0);                 // Green
        LCD::drawRect(10, 10, 220, 100, 0x001F); // Blue rectangle
        LCD::setBacklight(255);
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

    // Handle blink logic for outputs
    handleBlinkLogic();

    // Send periodic status report
    if (socketConnected && (millis() - lastStatusReport >= STATUS_REPORT_INTERVAL))
    {
        emitDevData();
        lastStatusReport = millis();
    }

    // Send status when state changed
    if (socketConnected && stateChanged)
    {
        emitDevData();
        stateChanged = false;
    }

    // Simple heartbeat check
    if (socketConnected && (millis() - lastPingTime > 70000))
    {
        DEBUG_PRINTLN("[SOCKET] Connection timeout, reconnecting...");
        socketConnected = false;
        connectSocketIO();
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
    JsonDocument doc;
    doc["sn"] = String(DEVICE_SN);
    doc["client_secret_key"] = String(CLIENT_SECRET_KEY);

    // sensorIds: 0x0f1234, 0x0f1235, ... (generated from GPIO outputs count)
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
    DEBUG_PRINTLN("\n[SOCKET] Connecting to Socket.IO...");

    // Extract domain from SERVER_URL
    String domain = String(SERVER_URL);
    domain.replace("https://", "");
    domain.replace("http://", "");

    // Build sensorIds JSON string
    String sensorIds = "[";
    for (size_t i = 0; i < SENSOR_COUNT; i++)
    {
        sensorIds += String((uint32_t)(BASE_SENSOR_ID + i));
        if (i + 1 < SENSOR_COUNT)
            sensorIds += ",";
    }
    sensorIds += "]";

    // Build Socket.IO path with query parameters (match Node.js client)
    String socketPath = String(API_PATH) +
                        "?sn=" + String(DEVICE_SN) +
                        "&clientType=device" +
                        "&clientVersion=V4" +
                        "&sensorIds=" + sensorIds +
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
            if (doc["sid"].is<const char *>())
            {
                socketSid = doc["sid"].as<String>();
                DEBUG_PRINTF("[SOCKET] SID: %s\n", socketSid.c_str());
            }

            // Send connect acknowledgment with auth token payload
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

                // Parse event
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, eventData);

                if (!error && doc.is<JsonArray>())
                {
                    JsonArray arr = doc.as<JsonArray>();
                    if (arr.size() >= 2)
                    {
                        String eventName = arr[0].as<String>();
                        DEBUG_PRINTF("[SOCKET] Event name: %s\n", eventName.c_str());

                        if (eventName == "app-cmd" || eventName == "appcmd")
                        {
                            // Extract command data
                            String cmdData;
                            serializeJson(arr[1], cmdData);
                            processAppCmd(cmdData);
                        }
                        else if (eventName == "connected")
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

    // Add GPIO states
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        content.add(gpioOutputs[i].state ? 1 : 0);
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

// ==================================================
// Process App Command
// ==================================================
void processAppCmd(const String &data)
{
    DEBUG_PRINTF("[CMD] Processing command: %s\n", data.c_str());

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error)
    {
        DEBUG_PRINTF("[CMD] JSON parse error: %s\n", error.c_str());
        return;
    }

    // Extract command parameters
    String cmd = "";
    int16_t index = -1;
    int8_t value = -1;

    if (doc["operation"].is<JsonObject>())
    {
        JsonObject operation = doc["operation"];

        if (operation["customCmd"].is<const char *>())
        {
            cmd = operation["customCmd"].as<String>();
        }
        if (operation["fieldIndex"].is<int>())
        {
            index = operation["fieldIndex"].as<int16_t>();
        }
        if (operation["fieldValue"].is<int>())
        {
            value = operation["fieldValue"].as<int8_t>();
        }
    }

    DEBUG_PRINTF("[CMD] Parsed - cmd: %s, index: %d, value: %d\n",
                 cmd.c_str(), index, value);

    // Process command
    if (cmd.length() == 0)
    {
        // Simple index/value control
        if (index >= 0 && index < 3 && value >= 0)
        {
            setState(index, value > 0);
        }
    }
    else
    {
        // Custom commands
        if (cmd == "output")
        {
            if (index >= 0 && index < 3 && value >= 0)
            {
                setState(index, value > 0);
            }
        }
        else if (cmd == "output-all")
        {
            if (value >= 0)
            {
                setStateAll(value > 0);
            }
        }
        else if (cmd == "blinkLed")
        {
            if (index >= 0 && index < 3)
            {
                setStateBlink(index, value > 0 ? value : 5);
            }
        }
        else if (cmd == "sync")
        {
            stateChanged = true; // Force status update
        }
        else if (cmd == "reboot")
        {
            DEBUG_PRINTLN("[CMD] Rebooting device...");
            emitDevStatus("Rebooting");
            delay(1000);
            ESP.restart();
        }
        else
        {
            DEBUG_PRINTF("[CMD] Unknown command: %s\n", cmd.c_str());
        }
    }
}

// ==================================================
// Set GPIO State
// ==================================================
void setState(uint8_t index, bool state)
{
    if (index < SENSOR_COUNT)
    {
        gpioOutputs[index].state = state;
        digitalWrite(gpioOutputs[index].pin, state ? HIGH : LOW);
        stateChanged = true;

        DEBUG_PRINTF("[GPIO] Pin %d set to %s\n",
                     gpioOutputs[index].pin,
                     state ? "ON" : "OFF");
    }
}

// ==================================================
// Set All GPIO States
// ==================================================
void setStateAll(bool state)
{
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        setState(i, state);
    }
    DEBUG_PRINTF("[GPIO] All pins set to %s\n", state ? "ON" : "OFF");
}

// ==================================================
// Set GPIO Blink
// ==================================================
void setStateBlink(uint8_t index, uint16_t count)
{
    if (index < SENSOR_COUNT)
    {
        gpioOutputs[index].blinkCount = count * 2; // *2 for on+off cycles
        DEBUG_PRINTF("[GPIO] Pin %d blink started (count: %d)\n",
                     gpioOutputs[index].pin, count);
    }
}

// ==================================================
// Handle Blink Logic
// ==================================================
void handleBlinkLogic()
{
    if (millis() - lastBlinkToggle < BLINK_INTERVAL)
    {
        return;
    }

    lastBlinkToggle = millis();

    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        if (gpioOutputs[i].blinkCount > 0)
        {
            setState(i, !gpioOutputs[i].state);
            gpioOutputs[i].blinkCount--;

            if (gpioOutputs[i].blinkCount == 0)
            {
                setState(i, false); // Ensure OFF when done
                DEBUG_PRINTF("[GPIO] Pin %d blink completed\n", gpioOutputs[i].pin);
            }
        }
    }
}
