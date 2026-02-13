#include "config.h"

#ifdef HAS_LCD_240x320

#include "lcd.h"
#include <LovyanGFX.hpp>

// LovyanGFX-based implementation that preserves the existing LCD:: API.
// This acts as a thin wrapper so existing code doesn't need to change.

// Custom minimal LGFX config for our ST7789 module and custom pins
class LGFX_Config : public lgfx::LGFX_Device
{
public:
    LGFX_Config()
    {
        // SPI bus configuration
        auto bus_cfg = _bus_spi.config();
        bus_cfg.spi_host = VSPI_HOST;  // use VSPI or HSPI as desired
        bus_cfg.freq_write = 10000000; // 10MHz (safer for some modules)
        bus_cfg.freq_read = 4000000;
        bus_cfg.pin_sclk = LCD_SCK_PIN;
        bus_cfg.pin_mosi = LCD_MOSI_PIN;
        bus_cfg.pin_miso = LCD_MISO_PIN;
        bus_cfg.pin_dc = LCD_DC_PIN;
        _bus_spi.config(bus_cfg);

        // Attach SPI bus to the panel (missing before: without this, LovyanGFX draw calls are no-ops)
        _panel_st7789.setBus(&_bus_spi);

        // Panel configuration (ST7789)
        auto panel_cfg = _panel_st7789.config();
        panel_cfg.pin_cs = LCD_CS_PIN;
        panel_cfg.pin_rst = LCD_RST_PIN;
        panel_cfg.pin_busy = -1;
        panel_cfg.panel_width = 240;
        panel_cfg.panel_height = 320;
        panel_cfg.offset_rotation = 0;
        panel_cfg.offset_x = 0;
        panel_cfg.offset_y = 0;
        // Use standard RGB order; BGR + swapped bytes caused incorrect colors on this panel
        panel_cfg.rgb_order = false;
        _panel_st7789.config(panel_cfg);

        setPanel(&_panel_st7789);
    }

private:
    lgfx::Bus_SPI _bus_spi;
    lgfx::Panel_ST7789P3 _panel_st7789;
};

static LGFX_Config display;

void LCD::begin()
{
    // If available, perform a manual hardware reset sequence first (some modules need it)
    if (LCD_RST_PIN >= 0)
    {
        pinMode(LCD_RST_PIN, OUTPUT);
        digitalWrite(LCD_RST_PIN, LOW);
        delay(20);
        digitalWrite(LCD_RST_PIN, HIGH);
        delay(20);
#if DEBUG_ENABLED
        Serial.println("[LCD] Manual RST toggle done");
#endif
    }

#if DEBUG_ENABLED
    Serial.println("[LCD] Initializing LovyanGFX panel (freq_write=20MHz)");
#endif
    // Initialize panel
    display.init();

    // Leave byte order as-is; swapping plus BGR was causing blue-only output
    display.setSwapBytes(false);

    // ensure backlight PWM initialized
    pinMode(LCD_BL_PIN, OUTPUT);
    ledcSetup(0, 5000, 8);
    ledcAttachPin(LCD_BL_PIN, 0);
    ledcWrite(0, 255);
    // also drive BL pin high directly to ensure backlight (useful for modules with simple BL pin)
    digitalWrite(LCD_BL_PIN, HIGH);

    // brief probe-read of pins for diagnostics (mirrors the serial output in main)
    if (LCD_BL_PIN >= 0)
    {
        pinMode(LCD_BL_PIN, INPUT_PULLUP);
        int bl = digitalRead(LCD_BL_PIN);
        pinMode(LCD_BL_PIN, OUTPUT);
#if DEBUG_ENABLED
        Serial.printf("[LCD-DIAG] BL pin read back = %d\n", bl);
#endif
    }
}

void LCD::setBacklight(uint8_t level)
{
    ledcWrite(0, level);
    digitalWrite(LCD_BL_PIN, level > 0 ? HIGH : LOW);
}

void LCD::setRotation(uint8_t rot)
{
    // LovyanGFX uses 0-3 for rotation
    display.setRotation(rot & 3);
}

void LCD::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    // LovyanGFX expects 16-bit RGB565 color; pass through
    display.drawPixel(x, y, color);
}

void LCD::fillScreen(uint16_t color)
{
    display.fillScreen(color);
}

void LCD::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    display.fillRect(x, y, w, h, color);
}

void LCD::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    display.drawRect(x, y, w, h, color);
}

#endif // HAS_LCD_240x320
