// Lightweight UI helpers for the 240x320 LCD panel.
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

#ifdef HAS_LCD_240x320

struct UiSnapshot
{
    bool wifiConnected;
    int wifiRssi; // dBm, valid only when wifiConnected is true
    bool socketConnected;
    bool sensors[SENSOR_COUNT]; // true = ON/active
    String dateText;            // e.g., "2026-02-13"
    String timeText;            // e.g., "10:24:30"
    IPAddress ip;
};

// Initialize the LCD surface and paint the static chrome.
void lcdUiInit();

// Render / refresh the LCD UI with the provided snapshot.
void lcdUiRender(const UiSnapshot &state);

#endif // HAS_LCD_240x320