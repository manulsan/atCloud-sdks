#include "socketio_client.h"
#include "config.h"

SocketIOClient *SocketIOClient::instance = nullptr;

SocketIOClient::SocketIOClient()
{
    instance = this;
}

void SocketIOClient::begin(const String &token)
{
    // Extract domain from SERVER_URL
    String domain = String(SERVER_URL);
    domain.replace("https://", "");
    domain.replace("http://", "");

    // Build sensorIds JSON string like Node.js client
    String sensorIds = "[";
    for (size_t i = 0; i < SENSOR_COUNT; i++)
    {
        sensorIds += String((uint32_t)(BASE_SENSOR_ID + i));
        if (i + 1 < SENSOR_COUNT)
            sensorIds += ",";
    }
    sensorIds += "]";

    String socketPath = String(API_PATH) +
                        "?sn=" + String(DEVICE_SN) +
                        "&clientType=device" +
                        "&clientVersion=V4" +
                        "&sensorIds=" + sensorIds +
                        "&EIO=4&transport=websocket";

    ws.beginSSL(domain.c_str(), SERVER_PORT, socketPath.c_str());
    ws.onEvent(SocketIOClient::wsEventStatic);
    ws.setReconnectInterval(reconnectInterval);
}

void SocketIOClient::loop()
{
    ws.loop();
}

void SocketIOClient::sendPacket(const char *type, const String &data)
{
    String packet = String(type);
    if (data.length() > 0)
        packet += data;
    ws.sendTXT(packet);
}

void SocketIOClient::wsEventStatic(WStype_t type, uint8_t *payload, size_t length)
{
    if (instance)
        instance->wsEvent(type, payload, length);
}

void SocketIOClient::wsEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        if (disconnectCb)
            disconnectCb();
        break;

    case WStype_CONNECTED:
        if (connectCb)
            connectCb();
        break;

    case WStype_TEXT:
        if (packetCb && payload && length > 0)
            packetCb((const char *)payload, length);
        break;

    case WStype_ERROR:
        DEBUG_PRINTLN("[SOCKET] WebSocket error");
        break;

    default:
        break;
    }
}
