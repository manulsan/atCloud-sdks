#ifndef LCD_H
#define LCD_H

#include <Arduino.h>

#ifdef HAS_LCD_240x320

class LCD
{
public:
    static void begin();
    static void setBacklight(uint8_t level); // 0..255
    static void fillScreen(uint16_t color);
    static void drawPixel(int16_t x, int16_t y, uint16_t color);
    static void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    static void setRotation(uint8_t rot); // 0-3

private:
    static void writeCommand(uint8_t cmd);
    static void writeData(uint8_t data);
    static void writeDataBuffer(const uint8_t *buf, size_t len);
    static void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    static inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
};

#endif // HAS_LCD_240x320

#endif // LCD_H
