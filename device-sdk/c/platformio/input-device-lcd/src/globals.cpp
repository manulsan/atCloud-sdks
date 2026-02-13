#include "main.h"

// Global variable definitions (single translation unit)
bool socketConnected = false;
bool bootupReady = false;
String authToken = "";
String socketSid = "";

GpioState gpioInputs[SENSOR_COUNT] = {
    {GPIO_INPUT_1, false, false},
    {GPIO_INPUT_2, false, false},
    {GPIO_INPUT_3, false, false}};

unsigned long lastGpioScan = 0;
unsigned long lastDataSend = 0;
bool dataUpdateRequired = true; // send update on boot
