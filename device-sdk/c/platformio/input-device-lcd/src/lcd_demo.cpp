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
static void lcdDrawText(const char *text, int x, int y, uint16_t color, uint8_t scale = 2)
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

void setup()
{
    Serial.begin(115200);
    delay(100);

    Serial.println("[LCD-DEMO] Initializing LCD...");
    LCD::begin();
    LCD::fillScreen(0x001F);                 // blue background
    LCD::drawRect(10, 10, 220, 100, 0xF800); // red rectangle
    LCD::setBacklight(255);

    // show demo text
    lcdDrawText("Hello", 60, 40, 0xFFFF, 2);

    Serial.println("[LCD-DEMO] Done.");
}

void loop() {}
