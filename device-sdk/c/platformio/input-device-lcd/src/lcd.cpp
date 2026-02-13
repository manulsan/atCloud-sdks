#ifdef HAS_LCD_240x320

#include "lcd.h"
#include "config.h"
#include <SPI.h>

// ST7789P3 command set (subset)
#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT 0x11
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_PASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_MADCTL 0x36
#define ST7789_COLMOD 0x3A

// Display dimensions for ST7789P3 (assumed 240x320 module)
static const uint16_t ST_WIDTH = 240;
static const uint16_t ST_HEIGHT = 320;

void LCD::writeCommand(uint8_t cmd)
{
    digitalWrite(LCD_DC_PIN, LOW);
    digitalWrite(LCD_CS_PIN, LOW);
    SPI.transfer(cmd);
    digitalWrite(LCD_CS_PIN, HIGH);
}

void LCD::writeData(uint8_t data)
{
    digitalWrite(LCD_DC_PIN, HIGH);
    digitalWrite(LCD_CS_PIN, LOW);
    SPI.transfer(data);
    digitalWrite(LCD_CS_PIN, HIGH);
}

void LCD::writeDataBuffer(const uint8_t *buf, size_t len)
{
    digitalWrite(LCD_DC_PIN, HIGH);
    digitalWrite(LCD_CS_PIN, LOW);
    for (size_t i = 0; i < len; ++i)
        SPI.transfer(buf[i]);
    digitalWrite(LCD_CS_PIN, HIGH);
}

void LCD::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // Column address set
    writeCommand(ST7789_CASET);
    writeData(x0 >> 8);
    writeData(x0 & 0xFF);
    writeData(x1 >> 8);
    writeData(x1 & 0xFF);

    // Row address set
    writeCommand(ST7789_PASET);
    writeData(y0 >> 8);
    writeData(y0 & 0xFF);
    writeData(y1 >> 8);
    writeData(y1 & 0xFF);

    // Write to RAM
    writeCommand(ST7789_RAMWR);
}

void LCD::begin()
{
    pinMode(LCD_CS_PIN, OUTPUT);
    pinMode(LCD_DC_PIN, OUTPUT);
    pinMode(LCD_RST_PIN, OUTPUT);
    pinMode(LCD_BL_PIN, OUTPUT);

    // Initialize SPI with specified pins
    SPI.begin(LCD_SCK_PIN, LCD_MISO_PIN, LCD_MOSI_PIN, LCD_CS_PIN);
    digitalWrite(LCD_CS_PIN, HIGH);

    // hardware reset
    digitalWrite(LCD_RST_PIN, HIGH);
    delay(5);
    digitalWrite(LCD_RST_PIN, LOW);
    delay(20);
    digitalWrite(LCD_RST_PIN, HIGH);
    delay(150);

    // Software reset
    writeCommand(ST7789_SWRESET);
    delay(150);

    // Sleep out
    writeCommand(ST7789_SLPOUT);
    delay(120);

    // Interface pixel format: 16 bits/pixel (RGB565)
    writeCommand(ST7789_COLMOD);
    writeData(0x55);
    delay(10);

    // Memory data access control (orientation + RGB order)
    // 0x00 - RGB default; adjust if colors/orientation are wrong for your module
    writeCommand(ST7789_MADCTL);
    writeData(0x00);
    delay(10);

    // Display ON
    writeCommand(ST7789_DISPON);
    delay(100);

    // set backlight on (use LEDC PWM)
    ledcSetup(0, 5000, 8);
    ledcAttachPin(LCD_BL_PIN, 0);
    ledcWrite(0, 255);
}

void LCD::setBacklight(uint8_t level)
{
    // level: 0..255
    ledcWrite(0, level);
}

void LCD::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || x >= ST_WIDTH || y < 0 || y >= ST_HEIGHT)
        return;

    setAddrWindow(x, y, x, y);
    writeData(color >> 8);
    writeData(color & 0xFF);
}

void LCD::fillScreen(uint16_t color)
{
    const uint16_t w = ST_WIDTH;
    const uint16_t h = ST_HEIGHT;
    setAddrWindow(0, 0, w - 1, h - 1);

    // send pixel data
    digitalWrite(LCD_DC_PIN, HIGH);
    digitalWrite(LCD_CS_PIN, LOW);
    for (uint32_t i = 0; i < (uint32_t)w * h; ++i)
    {
        SPI.transfer(color >> 8);
        SPI.transfer(color & 0xFF);
    }
    digitalWrite(LCD_CS_PIN, HIGH);
}

void LCD::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // Clip rect to display bounds
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > ST_WIDTH)
        w = ST_WIDTH - x;
    if (y + h > ST_HEIGHT)
        h = ST_HEIGHT - y;

    for (int16_t i = 0; i < w; ++i)
    {
        drawPixel(x + i, y, color);
        drawPixel(x + i, y + h - 1, color);
    }
    for (int16_t i = 0; i < h; ++i)
    {
        drawPixel(x, y + i, color);
        drawPixel(x + w - 1, y + i, color);
    }
}

#endif // HAS_LCD_240x320
