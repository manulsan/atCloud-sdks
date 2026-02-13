#ifndef SOCKETIO_CLIENT_H
#define SOCKETIO_CLIENT_H

#include <Arduino.h>
#include <WebSocketsClient.h>

// Simple Socket.IO client wrapper exposing callbacks for application code.
class SocketIOClient
{
public:
    using PacketCallback = void (*)(const char *payload, size_t length);
    using ConnectCallback = void (*)(void);
    using DisconnectCallback = void (*)(void);

    SocketIOClient();

    // Start connection using token (builds URL from config.h macros)
    void begin(const String &token);

    // Must be called regularly from main loop
    void loop();

    // Send a text packet (Socket.IO encoded string)
    void sendPacket(const char *type, const String &data = "");

    // Callbacks
    void setPacketCallback(PacketCallback cb) { packetCb = cb; }
    void setConnectCallback(ConnectCallback cb) { connectCb = cb; }
    void setDisconnectCallback(DisconnectCallback cb) { disconnectCb = cb; }

    // Reconnect interval (ms)
    void setReconnectInterval(unsigned long ms) { reconnectInterval = ms; }

private:
    WebSocketsClient ws;
    PacketCallback packetCb = nullptr;
    ConnectCallback connectCb = nullptr;
    DisconnectCallback disconnectCb = nullptr;
    unsigned long reconnectInterval = 5000;

    static void wsEventStatic(WStype_t type, uint8_t *payload, size_t length);
    void wsEvent(WStype_t type, uint8_t *payload, size_t length);

    // singleton pointer used by static event wrapper
    static SocketIOClient *instance;
};

#endif // SOCKETIO_CLIENT_H
