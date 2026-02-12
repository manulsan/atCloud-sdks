# atCloud365 Input Device LCD (PlatformIO)

Small standalone project containing only the LCD driver/demo for ST7789P3 (ESP32-32E module).

## Purpose
- Keep all LCD-related code (driver + demo) separate from `input-device` main project.
- Provides a focused place to develop and test display features.

## How to build
- Open this folder in VS Code (PlatformIO) or use PlatformIO CLI.
- Build & upload using `esp32dev` environment.

## Demo
- `src/lcd_demo.cpp` — initializes the ST7789P3 and draws a demo "Hello" on screen.

## Pin mapping (module default)
- `LCD_SCK_PIN` = IO14
- `LCD_MOSI_PIN` = IO13
- `LCD_MISO_PIN` = IO12
- `LCD_CS_PIN`   = IO15
- `LCD_DC_PIN`   = IO2
- `LCD_RST_PIN`  = IO22
- `LCD_BL_PIN`   = IO21

> https://www.lcdwiki.com/2.8inch_ESP32-32E-7789