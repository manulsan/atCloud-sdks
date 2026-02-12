#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include "config.h"

// GPIO state structure
struct GpioState
{
    uint8_t pin;
    bool state;
    bool previousState;
};

// Shared globals (defined in main.cpp)
extern WebSocketsClient webSocket;
extern bool socketConnected;
extern bool bootupReady;
extern String authToken;
extern String socketSid;
extern GpioState gpioInputs[];
extern unsigned long lastGpioScan;
extern unsigned long lastDataSend;
extern unsigned long lastPingTime;
extern bool dataUpdateRequired;

// Function prototypes
void setupWiFi();
bool authenticateDevice();
void connectSocketIO();
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
void handleSocketIOPacket(const char *packet, size_t length);
void sendPacket(const char *type, const String &data = "");
void emitDevData();
void emitDevStatus(const String &status);
bool scanGpioInputs();

#endif // MAIN_H
