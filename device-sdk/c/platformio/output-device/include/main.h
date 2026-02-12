#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <WebSocketsClient.h>
#include "config.h"

// GPIO output structure
struct GpioOutput
{
    uint8_t pin;
    bool state;
    uint16_t blinkCount; // For blinking
};

// Shared globals (defined in main.cpp)
extern WebSocketsClient webSocket;
extern bool socketConnected;
extern bool bootupReady;
extern String authToken;
extern String socketSid;
extern GpioOutput gpioOutputs[];
extern unsigned long lastStatusReport;
extern unsigned long lastBlinkToggle;
extern unsigned long lastPingTime;
extern bool stateChanged;

// Function prototypes
void setupWiFi();
bool authenticateDevice();
void connectSocketIO();
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
void handleSocketIOPacket(const char *packet, size_t length);
void sendPacket(const char *type, const String &data = "");
void emitDevData();
void emitDevStatus(const String &status);
void processAppCmd(const String &data);
void setState(uint8_t index, bool state);
void setStateAll(bool state);
void setStateBlink(uint8_t index, uint16_t count = 5);
void handleBlinkLogic();

#endif // MAIN_H