/**
 * @file st7735.h
 * @brief ST7735 TFT LCD Driver for Daisy
 * 
 * Compatible with OledDisplay template, drop-in replacement for SSD130x
 * Supports RGB565 color (65K colors)
 * 
 * Default Wiring (Daisy Seed / Daisy Patch compatible):
 *   SCL  -> D8  (SPI1_SCK  / PG11)
 *   SDA  -> D10 (SPI1_MOSI / PB5)
 *   DC   -> D9  (PB4)
 *   CS   -> D7  (PA2) - Software CS
 *   RES  -> D30 (PB15)
 */

#pragma once
#ifndef DSY_ST7735_H
#define DSY_ST7735_H

#include "daisy_seed.h"

namespace daisy
{

/**
 * 4 Wire SPI Transport for ST7735 TFT display devices
 */
class ST7735_4WireSpiTransport
{
  public:
    struct Config
    {
        struct
        {
            Pin dc;
            Pin reset;
            Pin cs;
        } pin_config;
        
        void Defaults()
        {
            pin_config.dc    = seed::D9;
            pin_config.reset = seed::D30;
            pin_config.cs    = seed::D7;
        }
    };
    
    void Init(const Config& config)
    {
        pin_dc_.Init(config.pin_config.dc, GPIO::Mode::OUTPUT);
        pin_cs_.Init(config.pin_config.cs, GPIO::Mode::OUTPUT);
        pin_reset_.Init(config.pin_config.reset, GPIO::Mode::OUTPUT);
        
        pin_dc_.Write(true);
        pin_cs_.Write(true);
        pin_reset_.Write(true);
        
        SpiHandle::Config spi_config;
        spi_config.periph         = SpiHandle::Config::Peripheral::SPI_1;
        spi_config.mode           = SpiHandle::Config::Mode::MASTER;
        spi_config.direction      = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
        spi_config.datasize       = 8;
        spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
        spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
        spi_config.nss            = SpiHandle::Config::NSS::SOFT;
        spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_4;
        spi_config.pin_config.sclk = seed::D8;
        spi_config.pin_config.mosi = seed::D10;
        spi_config.pin_config.miso = Pin();
        spi_config.pin_config.nss  = Pin();
        
        spi_.Init(spi_config);
        
        System::Delay(10);
        
        pin_reset_.Write(true);
        System::Delay(10);
        pin_reset_.Write(false);
        System::Delay(10);
        pin_reset_.Write(true);
        System::Delay(120);
    }
    
    void SendCommand(uint8_t cmd)
    {
        pin_dc_.Write(false);
        pin_cs_.Write(false);
        spi_.BlockingTransmit(&cmd, 1);
        pin_cs_.Write(true);
    }
    
    void SendData(uint8_t* buff, size_t size)
    {
        pin_dc_.Write(true);
        pin_cs_.Write(false);
        spi_.BlockingTransmit(buff, size);
        pin_cs_.Write(true);
    }

  private:
    SpiHandle spi_;
    GPIO pin_reset_;
    GPIO pin_dc_;
    GPIO pin_cs_;
};


/**
 * A driver implementation for the ST7735 TFT LCD
 * API compatible with SSD130xDriver, with added color support
 */
template <size_t width, size_t height, typename Transport>
class ST7735Driver
{
  public:
    // RGB565 Color Constants - Rainbow Palette for Colorful UI
    static constexpr uint16_t COLOR_BLACK   = 0x0000;
    static constexpr uint16_t COLOR_WHITE   = 0xFFFF;
    static constexpr uint16_t COLOR_RED     = 0xF800;
    static constexpr uint16_t COLOR_GREEN   = 0x07E0;
    static constexpr uint16_t COLOR_BLUE    = 0x001F;
    static constexpr uint16_t COLOR_CYAN    = 0x07FF;
    static constexpr uint16_t COLOR_MAGENTA = 0xF81F;
    static constexpr uint16_t COLOR_YELLOW  = 0xFFE0;
    static constexpr uint16_t COLOR_ORANGE  = 0xFC00;
    static constexpr uint16_t COLOR_GRAY    = 0x8410;
    static constexpr uint16_t COLOR_PINK    = 0xF81F;
    static constexpr uint16_t COLOR_PURPLE  = 0x780F;
    static constexpr uint16_t COLOR_LIME    = 0x87E0;
    static constexpr uint16_t COLOR_NAVY    = 0x0010;
    static constexpr uint16_t COLOR_TEAL    = 0x0410;
    static constexpr uint16_t COLOR_BROWN   = 0x8200;
    static constexpr uint16_t COLOR_DARKGREEN = 0x0320;
    static constexpr uint16_t COLOR_DARKBLUE  = 0x0011;
    static constexpr uint16_t COLOR_SKYBLUE   = 0x5D1F;
    static constexpr uint16_t COLOR_GOLD      = 0xFEA0;
    
    // Convert RGB888 to RGB565
    static uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    struct Config
    {
        typename Transport::Config transport_config;
    };

    void Init(Config config)
    {
        transport_.Init(config.transport_config);
        
        // Initialize color palette for themes
        foreground_color_ = COLOR_WHITE;
        background_color_ = COLOR_BLACK;
        accent_color_ = COLOR_CYAN;
        
        transport_.SendCommand(0x01); // SWRESET
        System::Delay(150);
        
        transport_.SendCommand(0x11); // SLPOUT
        System::Delay(120);
        
        transport_.SendCommand(0xB1); // FRMCTR1
        uint8_t frmctr1[] = {0x01, 0x2C, 0x2D};
        transport_.SendData(frmctr1, 3);
        
        transport_.SendCommand(0xB2);
        transport_.SendData(frmctr1, 3);
        
        transport_.SendCommand(0xB3);
        uint8_t frmctr3[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D};
        transport_.SendData(frmctr3, 6);
        
        transport_.SendCommand(0xB4); // INVCTR
        uint8_t invctr = 0x07;
        transport_.SendData(&invctr, 1);
        
        transport_.SendCommand(0xC0); // PWCTR1
        uint8_t pwctr1[] = {0xA2, 0x02, 0x84};
        transport_.SendData(pwctr1, 3);
        
        transport_.SendCommand(0xC1);
        uint8_t pwctr2 = 0xC5;
        transport_.SendData(&pwctr2, 1);
        
        transport_.SendCommand(0xC2);
        uint8_t pwctr3[] = {0x0A, 0x00};
        transport_.SendData(pwctr3, 2);
        
        transport_.SendCommand(0xC3);
        uint8_t pwctr4[] = {0x8A, 0x2A};
        transport_.SendData(pwctr4, 2);
        
        transport_.SendCommand(0xC4);
        uint8_t pwctr5[] = {0x8A, 0xEE};
        transport_.SendData(pwctr5, 2);
        
        transport_.SendCommand(0xC5); // VMCTR1
        uint8_t vmctr1 = 0x0E;
        transport_.SendData(&vmctr1, 1);
        
        transport_.SendCommand(0x20); // INVOFF
        
        transport_.SendCommand(0x36); // MADCTL
        uint8_t madctl = 0xC8;
        transport_.SendData(&madctl, 1);
        
        transport_.SendCommand(0x3A); // COLMOD
        uint8_t colmod = 0x05; // 16-bit RGB565
        transport_.SendData(&colmod, 1);
        
        System::Delay(10);
        
        transport_.SendCommand(0xE0); // GMCTRP1
        uint8_t gmctrp1[] = {0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d,
                            0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10};
        transport_.SendData(gmctrp1, 16);
        
        transport_.SendCommand(0xE1); // GMCTRN1
        uint8_t gmctrn1[] = {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
                            0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10};
        transport_.SendData(gmctrn1, 16);
        
        transport_.SendCommand(0x13); // NORON
        System::Delay(10);
        
        transport_.SendCommand(0x29); // DISPON
        System::Delay(100);
        
        Fill(false);
        Update();
    }

    size_t Width() const { return width; }
    size_t Height() const { return height; }

    // ============ Color Theme Functions ============
    
    /** Set foreground (text) color */
    void SetForegroundColor(uint16_t color) { foreground_color_ = color; }
    
    /** Set background color */
    void SetBackgroundColor(uint16_t color) { background_color_ = color; }
    
    /** Set accent color (for highlights) */
    void SetAccentColor(uint16_t color) { accent_color_ = color; }
    
    /** Get current foreground color */
    uint16_t GetForegroundColor() const { return foreground_color_; }
    
    /** Get current background color */
    uint16_t GetBackgroundColor() const { return background_color_; }
    
    /** Get current accent color */
    uint16_t GetAccentColor() const { return accent_color_; }
    
    /** Set a complete color theme */
    void SetTheme(uint16_t fg, uint16_t bg, uint16_t accent)
    {
        foreground_color_ = fg;
        background_color_ = bg;
        accent_color_ = accent;
    }
    
    /** Pre-defined themes */
    void SetThemeDefault()    { SetTheme(COLOR_WHITE, COLOR_BLACK, COLOR_CYAN); }
    void SetThemeCyberpunk()  { SetTheme(COLOR_CYAN, COLOR_DARKBLUE, COLOR_MAGENTA); }
    void SetThemeMatrix()     { SetTheme(COLOR_GREEN, COLOR_BLACK, COLOR_LIME); }
    void SetThemeSunset()     { SetTheme(COLOR_ORANGE, COLOR_PURPLE, COLOR_YELLOW); }
    void SetThemeOcean()      { SetTheme(COLOR_SKYBLUE, COLOR_NAVY, COLOR_CYAN); }
    void SetThemeRetro()      { SetTheme(COLOR_YELLOW, COLOR_BROWN, COLOR_ORANGE); }
    void SetThemeNeon()       { SetTheme(COLOR_MAGENTA, COLOR_BLACK, COLOR_CYAN); }

    // ============ SSD130x Compatible Drawing Functions ============
    
    /** Draw a pixel (SSD130x compatible - uses theme colors) */
    void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on)
    {
        DrawPixelColor(x, y, on ? foreground_color_ : background_color_);
    }

    /** Draw a pixel with RGB565 color */
    void DrawPixelColor(uint_fast8_t x, uint_fast8_t y, uint16_t color)
    {
        if(x >= width || y >= height)
            return;
        size_t idx = (y * width + x) * 2;
        buffer_[idx]     = color >> 8;
        buffer_[idx + 1] = color & 0xFF;
    }

    /** Draw a pixel with RGB values (0-255) */
    void DrawPixelRGB(uint_fast8_t x, uint_fast8_t y, uint8_t r, uint8_t g, uint8_t b)
    {
        DrawPixelColor(x, y, RGB565(r, g, b));
    }

    /** Fill entire display (SSD130x compatible - uses theme colors) */
    void Fill(bool on)
    {
        FillColor(on ? foreground_color_ : background_color_);
    }

    /** Fill entire display with RGB565 color */
    void FillColor(uint16_t color)
    {
        uint8_t hi = color >> 8;
        uint8_t lo = color & 0xFF;
        for(size_t i = 0; i < sizeof(buffer_); i += 2)
        {
            buffer_[i]     = hi;
            buffer_[i + 1] = lo;
        }
    }

    /** Draw a filled rectangle with color */
    void DrawRectFilled(uint_fast8_t x, uint_fast8_t y, 
                        uint_fast8_t w, uint_fast8_t h, 
                        uint16_t color)
    {
        for(uint_fast8_t j = y; j < y + h && j < height; j++)
            for(uint_fast8_t i = x; i < x + w && i < width; i++)
                DrawPixelColor(i, j, color);
    }

    /** Draw rectangle outline (SSD130x compatible) */
    void DrawRect(uint_fast8_t x1, uint_fast8_t y1, 
                  uint_fast8_t x2, uint_fast8_t y2, bool on)
    {
        DrawRectColor(x1, y1, x2, y2, on ? foreground_color_ : background_color_);
    }
    
    /** Draw rectangle outline with color */
    void DrawRectColor(uint_fast8_t x1, uint_fast8_t y1, 
                       uint_fast8_t x2, uint_fast8_t y2, uint16_t color)
    {
        DrawHLineColor(x1, y1, x2 - x1 + 1, color);
        DrawHLineColor(x1, y2, x2 - x1 + 1, color);
        DrawVLineColor(x1, y1, y2 - y1 + 1, color);
        DrawVLineColor(x2, y1, y2 - y1 + 1, color);
    }

    /** Draw a horizontal line (SSD130x compatible) */
    void DrawHLine(uint_fast8_t x, uint_fast8_t y, uint_fast8_t w, bool on)
    {
        DrawHLineColor(x, y, w, on ? foreground_color_ : background_color_);
    }

    /** Draw a horizontal line with color */
    void DrawHLineColor(uint_fast8_t x, uint_fast8_t y, uint_fast8_t w, uint16_t color)
    {
        for(uint_fast8_t i = x; i < x + w && i < width; i++)
            DrawPixelColor(i, y, color);
    }

    /** Draw a vertical line (SSD130x compatible) */
    void DrawVLine(uint_fast8_t x, uint_fast8_t y, uint_fast8_t h, bool on)
    {
        DrawVLineColor(x, y, h, on ? foreground_color_ : background_color_);
    }

    /** Draw a vertical line with color */
    void DrawVLineColor(uint_fast8_t x, uint_fast8_t y, uint_fast8_t h, uint16_t color)
    {
        for(uint_fast8_t j = y; j < y + h && j < height; j++)
            DrawPixelColor(x, j, color);
    }
    
    /** Draw a line (SSD130x compatible) */
    void DrawLine(uint_fast8_t x1, uint_fast8_t y1,
                  uint_fast8_t x2, uint_fast8_t y2, bool on)
    {
        DrawLineColor(x1, y1, x2, y2, on ? foreground_color_ : background_color_);
    }
    
    /** Draw a line with color */
    void DrawLineColor(uint_fast8_t x1, uint_fast8_t y1,
                       uint_fast8_t x2, uint_fast8_t y2, uint16_t color)
    {
        int dx = abs((int)x2 - (int)x1);
        int dy = abs((int)y2 - (int)y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;
        
        while(true)
        {
            DrawPixelColor(x1, y1, color);
            if(x1 == x2 && y1 == y2) break;
            int e2 = 2 * err;
            if(e2 > -dy) { err -= dy; x1 += sx; }
            if(e2 < dx) { err += dx; y1 += sy; }
        }
    }

    /** Update the display */
    void Update()
    {
        transport_.SendCommand(0x2A);
        uint8_t caset[] = {0x00, 0x00, 0x00, (uint8_t)(width - 1)};
        transport_.SendData(caset, 4);
        
        transport_.SendCommand(0x2B);
        uint8_t raset[] = {0x00, 0x00, 0x00, (uint8_t)(height - 1)};
        transport_.SendData(raset, 4);
        
        transport_.SendCommand(0x2C);
        transport_.SendData(buffer_, sizeof(buffer_));
    }

  private:
    Transport transport_;
    uint8_t buffer_[width * height * 2];
    
    // Theme colors
    uint16_t foreground_color_;
    uint16_t background_color_;
    uint16_t accent_color_;
};

// Pre-defined driver types
using ST7735_4WireSpi128x160Driver = ST7735Driver<128, 160, ST7735_4WireSpiTransport>;
using ST7735_4WireSpi128x128Driver = ST7735Driver<128, 128, ST7735_4WireSpiTransport>;
using ST7735_4WireSpi80x160Driver  = ST7735Driver<80, 160, ST7735_4WireSpiTransport>;
using ST7735_4WireSpi128x64Driver  = ST7735Driver<128, 64, ST7735_4WireSpiTransport>;

} // namespace daisy

#endif // DSY_ST7735_H
