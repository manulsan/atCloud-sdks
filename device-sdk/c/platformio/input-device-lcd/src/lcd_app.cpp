#include <Arduino.h>
#include "lcd_app.h"
#include "lcd.h"
#include "main.h"

#ifdef HAS_LCD_240x320

// ------------------------------------------------------------
// Small 5x7 bitmap font for uppercase letters, digits, and a
// handful of punctuation. Stored column-wise.
// ------------------------------------------------------------
static const uint8_t FONT_5X7[][5] = {
    // 32 ' '
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // 33 '!'
    {0x00, 0x00, 0x5f, 0x00, 0x00},
    // 34 '"'
    {0x00, 0x07, 0x00, 0x07, 0x00},
    // 35 '#'
    {0x14, 0x7f, 0x14, 0x7f, 0x14},
    // 36 '$'
    {0x24, 0x2a, 0x7f, 0x2a, 0x12},
    // 37 '%'
    {0x23, 0x13, 0x08, 0x64, 0x62},
    // 38 '&'
    {0x36, 0x49, 0x55, 0x22, 0x50},
    // 39 '\''
    {0x00, 0x05, 0x03, 0x00, 0x00},
    // 40 '('
    {0x00, 0x1c, 0x22, 0x41, 0x00},
    // 41 ')'
    {0x00, 0x41, 0x22, 0x1c, 0x00},
    // 42 '*'
    {0x14, 0x08, 0x3e, 0x08, 0x14},
    // 43 '+'
    {0x08, 0x08, 0x3e, 0x08, 0x08},
    // 44 ','
    {0x00, 0x50, 0x30, 0x00, 0x00},
    // 45 '-'
    {0x08, 0x08, 0x08, 0x08, 0x08},
    // 46 '.'
    {0x00, 0x60, 0x60, 0x00, 0x00},
    // 47 '/'
    {0x20, 0x10, 0x08, 0x04, 0x02},
    // 48 '0'
    {0x3e, 0x45, 0x49, 0x51, 0x3e},
    // 49 '1'
    {0x00, 0x21, 0x7f, 0x01, 0x00},
    // 50 '2'
    {0x21, 0x43, 0x45, 0x49, 0x31},
    // 51 '3'
    {0x42, 0x41, 0x51, 0x69, 0x46},
    // 52 '4'
    {0x0c, 0x14, 0x24, 0x7f, 0x04},
    // 53 '5'
    {0x72, 0x51, 0x51, 0x51, 0x4e},
    // 54 '6'
    {0x1e, 0x29, 0x49, 0x49, 0x06},
    // 55 '7'
    {0x40, 0x47, 0x48, 0x50, 0x60},
    // 56 '8'
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // 57 '9'
    {0x30, 0x49, 0x49, 0x4a, 0x3c},
    // 58 ':'
    {0x00, 0x36, 0x36, 0x00, 0x00},
    // 59 ';'
    {0x00, 0x56, 0x36, 0x00, 0x00},
    // 60 '<'
    {0x08, 0x14, 0x22, 0x41, 0x00},
    // 61 '='
    {0x14, 0x14, 0x14, 0x14, 0x14},
    // 62 '>'
    {0x00, 0x41, 0x22, 0x14, 0x08},
    // 63 '?'
    {0x20, 0x40, 0x45, 0x48, 0x30},
    // 64 '@'
    {0x32, 0x49, 0x79, 0x41, 0x3e},
    // 65 'A'
    {0x3f, 0x48, 0x48, 0x48, 0x3f},
    // 66 'B'
    {0x7f, 0x49, 0x49, 0x49, 0x36},
    // 67 'C'
    {0x3e, 0x41, 0x41, 0x41, 0x22},
    // 68 'D'
    {0x7f, 0x41, 0x41, 0x22, 0x1c},
    // 69 'E'
    {0x7f, 0x49, 0x49, 0x49, 0x41},
    // 70 'F'
    {0x7f, 0x48, 0x48, 0x48, 0x40},
    // 71 'G'
    {0x3e, 0x41, 0x49, 0x49, 0x2e},
    // 72 'H'
    {0x7f, 0x08, 0x08, 0x08, 0x7f},
    // 73 'I'
    {0x00, 0x41, 0x7f, 0x41, 0x00},
    // 74 'J'
    {0x02, 0x01, 0x01, 0x01, 0x7e},
    // 75 'K'
    {0x7f, 0x08, 0x14, 0x22, 0x41},
    // 76 'L'
    {0x7f, 0x01, 0x01, 0x01, 0x01},
    // 77 'M'
    {0x7f, 0x20, 0x18, 0x20, 0x7f},
    // 78 'N'
    {0x7f, 0x10, 0x08, 0x04, 0x7f},
    // 79 'O'
    {0x3e, 0x41, 0x41, 0x41, 0x3e},
    // 80 'P'
    {0x7f, 0x48, 0x48, 0x48, 0x30},
    // 81 'Q'
    {0x3e, 0x41, 0x45, 0x42, 0x3d},
    // 82 'R'
    {0x7f, 0x48, 0x4c, 0x4a, 0x31},
    // 83 'S'
    {0x31, 0x49, 0x49, 0x49, 0x46},
    // 84 'T'
    {0x40, 0x40, 0x7f, 0x40, 0x40},
    // 85 'U'
    {0x7e, 0x01, 0x01, 0x01, 0x7e},
    // 86 'V'
    {0x7c, 0x02, 0x01, 0x02, 0x7c},
    // 87 'W'
    {0x7f, 0x02, 0x0c, 0x02, 0x7f},
    // 88 'X'
    {0x63, 0x14, 0x08, 0x14, 0x63},
    // 89 'Y'
    {0x70, 0x08, 0x07, 0x08, 0x70},
    // 90 'Z'
    {0x43, 0x45, 0x49, 0x51, 0x61},
};

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)((((uint16_t)(r & 0xF8) << 8)) | (((uint16_t)(g & 0xFC) << 3)) | ((b & 0xF8) >> 3));
}

// Simple Bresenham line drawer (used for tiny icons)
static void drawLine(int x0, int y0, int x1, int y1, uint16_t color)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        LCD::drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

// -------------------------------------------------------------------
// Font rendering
// -------------------------------------------------------------------
static const uint8_t *lookupGlyph(char c)
{
    // Normalize lowercase to uppercase to save table space
    if (c >= 'a' && c <= 'z')
        c = char(c - 'a' + 'A');

    if (c < 32 || c > 90)
        return FONT_5X7[0]; // space

    return FONT_5X7[c - 32];
}

void lcdDrawText(const char *text, int x, int y, uint16_t color, uint8_t scale)
{
    int cursorX = x;
    const int glyphW = 5 * scale;
    const int spacing = 1 * scale;

    for (const char *p = text; *p; ++p)
    {
        const uint8_t *glyph = lookupGlyph(*p);
        for (int col = 0; col < 5; ++col)
        {
            uint8_t bits = glyph[col];
            for (int row = 0; row < 7; ++row)
            {
                if (bits & (1 << row))
                {
                    LCD::fillRect(cursorX + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
        cursorX += glyphW + spacing;
    }
}

/*
    x- y coordinates are top-left of the LCD area
    320|
        |
        |
        |
        |
        +----------------240

*/
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
// -------------------------------------------------------------------
// Basic shapes and UI primitives
// -------------------------------------------------------------------
static void drawBadge(int x, int y, int w, int h, uint16_t bg)
{
    LCD::fillRect(x, y, w, h, bg);
}

static void drawWifiIcon(int x, int y, bool connected, int rssi)
{
    uint16_t color = connected ? rgb565(80, 200, 120) : rgb565(200, 80, 80);
    int bars = 0;
    if (connected)
    {
        if (rssi > -50)
            bars = 4;
        else if (rssi > -60)
            bars = 3;
        else if (rssi > -70)
            bars = 2;
        else
            bars = 1;
    }

    const int barW = 6;
    const int barSpacing = 4;
    for (int i = 0; i < 4; ++i)
    {
        int bx = x + i * (barW + barSpacing);
        int bh = 6 + i * 4;
        int by = y + (24 - bh);
        uint16_t c = (i < bars) ? color : rgb565(60, 60, 70);
        LCD::fillRect(bx, by, barW, bh, c);
    }

    // small base arc
    drawLine(x - 2, y + 24, x + 4, y + 24, color);
    drawLine(x + 4, y + 24, x + 10, y + 24, color);
}

// static void drawSocketNoConnectIcon(int x, int y, bool connected)
// {
//     uint16_t color = connected ? rgb565(80, 200, 255) : rgb565(200, 120, 80);
//     LCD::fillRect(x, y + 6, 22, 12, color);
//     LCD::fillRect(x + 4, y, 4, 6, color);
//     LCD::fillRect(x + 14, y, 4, 6, color);
//     LCD::fillRect(x + 6, y + 18, 10, 4, color);
//     LCD::drawRect(x, y + 6, 22, 12, rgb565(20, 30, 40));
// }
// Small cloud+plug motif for network/socket status.
static void drawSocketConnectIcon(int x, int y, bool connected)
{
    const uint16_t body = connected ? rgb565(90, 210, 255) : rgb565(200, 120, 80);
    const uint16_t outline = rgb565(20, 30, 40);

    // cloud puff (~20% bigger)
    LCD::fillRect(x + 1, y + 5, 14, 12, body);
    LCD::fillRect(x + 5, y + 1, 8, 8, body);
    LCD::fillRect(x - 1, y + 8, 6, 8, body);
    LCD::fillRect(x + 12, y + 8, 6, 8, body);
    LCD::drawRect(x + 1, y + 5, 14, 12, outline);

    // plug prongs (scaled up)
    LCD::fillRect(x + 6, y + 18, 2, 5, outline);
    LCD::fillRect(x + 10, y + 18, 2, 5, outline);

    // plug body (wider/taller)
    LCD::fillRect(x + 4, y + 14, 10, 5, outline);
}

static void drawSensorCard(int idx, bool on)
{
    const int cardX = 5;
    const int cardY = LCD_HEIGHT - 70 - (idx * 40);

    uint16_t bg = on ? rgb565(40, 90, 50) : rgb565(40, 44, 54);

    char label[24];
    snprintf(label, sizeof(label), "SENSOR %d:", idx + 1);
    lcdDrawText(label, cardX + 10, cardY + 10, rgb565(210, 220, 235), 2);

    const char *stateText = on ? "ON" : "OFF";
    uint16_t stateColor = on ? rgb565(200, 255, 200) : rgb565(220, 180, 160);
    // Clear previous state text area to avoid ghosting when toggling
    // LCD::fillRect(cardX + 140, cardY + 6, 90, 20, bg);
    drawBadge(cardX + 140, cardY + 6, 90, 20, rgb565(10, 12, 18));
    lcdDrawText(stateText, cardX + 150, cardY + 10, stateColor, 2);
}

static void drawHeader()
{
    const uint16_t headerBg = rgb565(18, 26, 36);
    // LCD::fillRect(0, 0, 240, 56, headerBg);
    drawBadge(5, 10, 230, 36, rgb565(10, 12, 18));
    // lcdDrawText("https://atcloud365.com", 10, 10, rgb565(210, 230, 255), 2);
    lcdDrawText("atcloud365.com", 10, 10, rgb565(210, 230, 255), 2);
}

// -------------------------------------------------------------------
// Public UI entry points
// -------------------------------------------------------------------
void lcdUiInit()
{
    LCD::fillScreen(rgb565(10, 12, 18));
    drawHeader();

    // Prepaint sensor slots so only state changes redraw later.
    for (int i = 0; i < SENSOR_COUNT; ++i)
    {
        drawSensorCard(i, false);
    }
}

void lcdUiRender(const UiSnapshot &state)
{
    static UiSnapshot prev = {};
    static bool first = true;

    if (first)
    {
        lcdUiInit();
        first = false;
    }

    // Header status row : date/time block (kept inside 240x320 with margin)
    const int badgeY = LCD_HEIGHT - 38; // 44px height + 10px bottom margin
    // Use the same color as the screen background to blend the badge
    drawBadge(5, badgeY, 100, 38, rgb565(10, 12, 18));
    lcdDrawText(state.dateText.c_str(), 10, badgeY + 10, rgb565(150, 190, 230), 1);
    lcdDrawText(state.timeText.c_str(), 10, badgeY + 22, rgb565(220, 240, 255), 2);

    drawWifiIcon(160, LCD_HEIGHT - 25, state.wifiConnected, state.wifiRssi);
    drawSocketConnectIcon(215, LCD_HEIGHT - 17, state.socketConnected);
#if 1
    // Sensors
    for (int i = 0; i < SENSOR_COUNT; ++i)
    {
        if (first || state.sensors[i] != prev.sensors[i])
        {
            //   Serial.printf("index %d state changed: %s\n", i + 1, state.sensors[i] ? "ON" : "OFF");
            drawSensorCard(i, state.sensors[i]);
        }

        prev.sensors[i] = state.sensors[i];
    }
#endif
    prev.wifiConnected = state.wifiConnected;
    prev.wifiRssi = state.wifiRssi;
    prev.socketConnected = state.socketConnected;
    prev.dateText = state.dateText;
    prev.timeText = state.timeText;
    prev.ip = state.ip;
}

#endif // HAS_LCD_240x320
