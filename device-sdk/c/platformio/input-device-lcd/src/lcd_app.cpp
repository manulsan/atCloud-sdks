#include <Arduino.h>
#include "lcd.h"
#include "config.h"
#include "main.h"

// --- Reused LCD helpers (moved from main.cpp) ---------------------------
// drawFilledRect is declared in include/main.h and used across the app.
void drawFilledRect(int x, int y, int w, int h, uint16_t color)
{
    for (int ix = 0; ix < w; ix++)
        for (int iy = 0; iy < h; iy++)
            LCD::drawPixel(x + ix, y + iy, color);
}

// Very small block-style glyph used by the public lcdDrawText() implementation.
static void lcdDrawCharBlock(int x, int y, char c, uint16_t color, uint8_t scale)
{
    switch (c)
    {
    case 'H':
        drawFilledRect(x, y, 3 * scale, 16 * scale, color);
        drawFilledRect(x + 8 * scale, y, 3 * scale, 16 * scale, color);
        drawFilledRect(x + 3 * scale, y + 6 * scale, 5 * scale, 3 * scale, color);
        break;

    case 'e':
        drawFilledRect(x, y + 2 * scale, 10 * scale, 3 * scale, color);
        drawFilledRect(x, y + 2 * scale, 3 * scale, 12 * scale, color);
        drawFilledRect(x + 3 * scale, y + 8 * scale, 7 * scale, 3 * scale, color);
        drawFilledRect(x, y + 14 * scale, 10 * scale, 3 * scale, color);
        break;

    case 'l':
        drawFilledRect(x + 3 * scale, y, 3 * scale, 16 * scale, color);
        break;

    case 'o':
        LCD::drawRect(x, y, 10 * scale, 16 * scale, color);
        drawFilledRect(x + 2 * scale, y + 2 * scale, 6 * scale, 12 * scale, color);
        break;

    case ' ':
        break;

    default:
        LCD::drawRect(x, y, 6 * scale, 10 * scale, color);
        break;
    }
}

// Public text helper (prototype already in include/main.h)
void lcdDrawText(const char *text, int x, int y, uint16_t color, uint8_t scale)
{
    int cursorX = x;
    for (const char *p = text; *p; ++p)
    {
        lcdDrawCharBlock(cursorX, y, *p, color, scale);
        cursorX += 12 * scale; // advance (letter width + spacing)
    }
}

// Helper to construct RGB565 color from 8-bit RGB
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)((((uint16_t)(r & 0xF8) << 8)) | (((uint16_t)(g & 0xFC) << 3)) | ((b & 0xF8) >> 3));
}

// Draw a grid of colored rectangles at an arbitrary origin (used to test position-specific issues)
static void drawColoredGridAt(int originX, int originY)
{
    const int cols = 4;
    const int rows = 3;
    const int pad = 6;
    const int cellW = 40; // fixed small cells for multi-grid layout
    const int cellH = 36;

    uint16_t colors[12] = {
        rgb565(255, 0, 0),     // Red
        rgb565(0, 255, 0),     // Green
        rgb565(0, 0, 255),     // Blue
        rgb565(255, 255, 0),   // Yellow
        rgb565(255, 0, 255),   // Magenta
        rgb565(0, 255, 255),   // Cyan
        rgb565(255, 165, 0),   // Orange
        rgb565(128, 0, 128),   // Purple
        rgb565(165, 42, 42),   // Brown-ish
        rgb565(192, 192, 192), // Light gray
        rgb565(255, 255, 255), // White
        rgb565(0, 0, 0)        // Black
    };

    int idx = 0;
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            int x = originX + c * (cellW + pad);
            int y = originY + r * (cellH + pad);
            drawFilledRect(x, y, cellW, cellH, colors[idx % 12]);
            LCD::drawRect(x, y, cellW, cellH, 0xFFFF);
            idx++;
        }
    }
}

// Convenience wrapper to draw the grid in three different panel locations
static void drawThreeGrids()
{
    // top-left, top-right, and lower-left
    drawColoredGridAt(6, 6);
    drawColoredGridAt(120, 6);
    drawColoredGridAt(6, 110);
}

// Public (file-local) app init/loop used by higher-level code if needed
static void lcdAppInit()
{
    Serial.begin(115200);
    delay(100);

    Serial.println("[LCD-APP] Initializing LCD...");
    LCD::begin();
    LCD::setBacklight(255);

    // Initial draw (rotation 0)
    LCD::setRotation(0);

    // Draw multiple colored rectangles across the panel at three positions
    LCD::fillScreen(0x0000);
    drawThreeGrids();

    // Small explicit RGB swatches (separate from the grids) for quick channel check
    drawFilledRect(200, 6, 10, 10, rgb565(255, 0, 0));  // Red swatch
    drawFilledRect(200, 22, 10, 10, rgb565(0, 255, 0)); // Green swatch
    drawFilledRect(200, 38, 10, 10, rgb565(0, 0, 255)); // Blue swatch

    // Draw header text after drawing/clearing so it's visible
    lcdDrawText("HELLO", 12, 150, 0xFFFF, 2);

    // Keep a small label below the grids
    lcdDrawText("Color grids (3 positions)", 12, 110, 0xFFFF, 2);

    Serial.println("[LCD-APP] Done.");
}

// Cycle rotations 0..3 and draw RGB stripes to verify color order
static void lcdAppLoop()
{
    static uint8_t rot = 0;
    static unsigned long lastChange = 0;
    const unsigned long interval = 1500; // ms

    if (millis() - lastChange < interval)
        return;

    lastChange = millis();
    rot = (rot + 1) & 3;
    LCD::setRotation(rot);

    // draw background and label
    LCD::fillScreen(0x0000);
    char buf[32];
    snprintf(buf, sizeof(buf), "Rotation: %u", rot);
    lcdDrawText(buf, 20, 10, 0xFFFF, 2);

    // Draw the colored-rectangle grids at three positions so you can compare the same color at different coordinates
    drawThreeGrids();
    lcdDrawText(buf, 20, 200, 0x07E0, 1);
}

// Keep local static renderer only; the global `lcdDrawText` used elsewhere remains
// implemented in `src/main.cpp`. If you want `lcdApp` to expose a public draw
// method, we can add an `LCDApp` class in a follow-up change.
