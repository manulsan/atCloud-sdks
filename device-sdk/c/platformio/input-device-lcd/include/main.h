#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include "socketio_client.h"
#include "config.h"

// GPIO state structure
struct GpioState
{
    uint8_t pin;
    bool state;
    bool previousState;
};

// Shared globals (defined in main.cpp)
extern SocketIOClient socketIo;
extern bool socketConnected;
extern bool bootupReady;
extern String authToken;
extern String socketSid;
extern GpioState gpioInputs[];
extern unsigned long lastGpioScan;
extern unsigned long lastDataSend;
extern bool dataUpdateRequired;

// Function prototypes
void setupWiFi();
bool authenticateDevice();
void connectSocketIO();
void handleSocketIOPacket(const char *packet, size_t length);
void sendPacket(const char *type, const String &data = "");
void emitDevData();
void emitDevStatus(const String &status);
bool scanGpioInputs();

// LCD helper (prototype)
void lcdDrawText(const char *text, int x, int y, uint16_t color, uint8_t scale = 2);

#endif // MAIN_H