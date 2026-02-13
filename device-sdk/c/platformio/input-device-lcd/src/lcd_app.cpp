#include <Arduino.h>
#include "lcd.h"
#include "config.h"

// Small helper to draw a filled rectangle (uses LCD::drawPixel)
static void drawFilledRect(int x, int y, int w, int h, uint16_t color)
{
    for (int ix = 0; ix < w; ix++)
        for (int iy = 0; iy < h; iy++)
            LCD::drawPixel(x + ix, y + iy, color);
}

// Very small block font for "Hello" (same style as original)
static void lcdDrawTextLocal(const char *text, int x, int y, uint16_t color, uint8_t scale = 2)
{
    int cursorX = x;
    for (const char *p = text; *p; ++p)
    {
        char c = *p;
        if (c == 'H')
        {
            drawFilledRect(cursorX, y, 3 * scale, 16 * scale, color);
            drawFilledRect(cursorX + 8 * scale, y, 3 * scale, 16 * scale, color);
            drawFilledRect(cursorX + 3 * scale, y + 6 * scale, 5 * scale, 3 * scale, color);
        }
        else if (c == 'e')
        {
            drawFilledRect(cursorX, y + 2 * scale, 10 * scale, 3 * scale, color);
            drawFilledRect(cursorX, y + 2 * scale, 3 * scale, 12 * scale, color);
            drawFilledRect(cursorX + 3 * scale, y + 8 * scale, 7 * scale, 3 * scale, color);
            drawFilledRect(cursorX, y + 14 * scale, 10 * scale, 3 * scale, color);
        }
        else if (c == 'l')
        {
            drawFilledRect(cursorX + 3 * scale, y, 3 * scale, 16 * scale, color);
        }
        else if (c == 'o')
        {
            LCD::drawRect(cursorX, y, 10 * scale, 16 * scale, color);
            drawFilledRect(cursorX + 2 * scale, y + 2 * scale, 6 * scale, 12 * scale, color);
        }
        cursorX += 12 * scale;
    }
}

// Public (file-local) app init/loop used by higher-level code if needed
static void lcdAppInit()
{
    Serial.begin(115200);
    delay(100);

    Serial.println("[LCD-APP] Initializing LCD...");
    LCD::begin();
    LCD::fillScreen(0x001F);                 // blue background
    LCD::drawRect(10, 10, 220, 100, 0xF800); // red rectangle
    LCD::setBacklight(255);

    // show demo text (use local renderer to avoid symbol collisions)
    lcdDrawTextLocal("Hello", 60, 40, 0xFFFF, 2);

    Serial.println("[LCD-APP] Done.");
}

static void lcdAppLoop() {}

// Keep local static renderer only; the global `lcdDrawText` used elsewhere remains
// implemented in `src/main.cpp`. If you want `lcdApp` to expose a public draw
// method, we can add an `LCDApp` class in a follow-up change.
