# ST7735 TFT Display Support for Oopsy

This adds support for **ST7735 RGB TFT displays** (128x160, 65K colors) to Oopsy, as a drop-in replacement for the standard SSD130x OLED displays.

## Features

- 🌈 **Full RGB565 Color Support** - 65,536 colors
- 🎨 **Colorful UI Themes** - Different color schemes for each display mode
- 🔌 **Daisy Patch Compatible Pinout** - Works with standard Daisy Patch wiring
- 📦 **Drop-in Replacement** - Same API as SSD130x, just change the target

## Color Themes by Mode

| Mode | Foreground | Background | Style |
|------|------------|------------|-------|
| **MENU** | Cyan | Dark Blue | Cyberpunk |
| **PARAMS** | Green | Black | Matrix |
| **SCOPE** | Orange | Purple | Sunset |
| **CONSOLE** | Magenta | Black | Neon |

## Hardware Wiring

### ST7735 Display Pinout (Daisy Seed)

| ST7735 Pin | Daisy Seed Pin | Function |
|------------|----------------|----------|
| VCC | 3V3 | Power (3.3V) |
| GND | GND | Ground |
| SCL/SCK | D8 (PG11) | SPI Clock |
| SDA/MOSI | D10 (PB5) | SPI Data |
| DC | D9 (PB4) | Data/Command |
| CS | D7 (PA2) | Chip Select |
| RST/RES | D30 (PB15) | Reset |
| BLK | 3V3 or PWM | Backlight |

### Encoder Pinout (Daisy Patch Compatible)

| Encoder Pin | Daisy Seed Pin |
|-------------|----------------|
| A | D12 |
| B | D11 |
| Click/SW | D0 |

### Knob Pinout

| Knob | Daisy Seed Pin |
|------|----------------|
| Knob 1 | D15 |
| Knob 2 | D16 |
| Knob 3 | D21 |
| Knob 4 | D18 |

## Usage with Oopsy

### 1. Select the ST7735 Target

In `oopsy.maxpat`, select **seed_st7735** from the Target dropdown menu.

### 2. Create Your gen~ Patch

Use standard gen~ with `Param` objects:

```
// Example gen~ code
Param gain(0.5, min=0, max=1);
Param frequency(440, min=20, max=2000);

in 1 left;
out 1 output;

output = left * gain;
```

### 3. Flash to Daisy

Click the **Flash** button in Oopsy. The display will show your parameters with colorful themes!

## Display Controls

- **Long Press Encoder** (>0.5s) - Open mode selection menu
- **Rotate Encoder** - Navigate / Select
- **Short Press Encoder** - Confirm selection

## Available Colors

The driver provides these predefined colors:

```cpp
COLOR_BLACK, COLOR_WHITE, COLOR_RED, COLOR_GREEN, COLOR_BLUE,
COLOR_CYAN, COLOR_MAGENTA, COLOR_YELLOW, COLOR_ORANGE, COLOR_GRAY,
COLOR_PINK, COLOR_PURPLE, COLOR_LIME, COLOR_NAVY, COLOR_TEAL,
COLOR_BROWN, COLOR_DARKGREEN, COLOR_DARKBLUE, COLOR_SKYBLUE, COLOR_GOLD
```

### Custom Colors

Use `RGB565(r, g, b)` to create custom colors:

```cpp
uint16_t myColor = Driver::RGB565(255, 128, 0); // Orange
```

## Files Modified/Added

### New Files
- `source/libdaisy/src/dev/st7735.h` - ST7735 driver
- `source/seed.st7735.json` - Oopsy target configuration

### Modified Files
- `source/oopsy.js` - Added ST7735 header detection and target path
- `source/genlib_daisy.h` - Added color theme switching
- `source/libdaisy/src/hid/disp/oled_display.h` - Added `GetDriver()` method
- `patchers/oopsy.maxpat` - Added seed_st7735 to target menu

## Customizing Themes

Edit `genlib_daisy.h` to change color themes:

```cpp
#ifdef OOPSY_TARGET_ST7735
{
    using Driver = daisy::ST7735_4WireSpi128x160Driver;
    switch(mode) {
        case MODE_MENU:    
            hardware.display.GetDriver().SetTheme(
                Driver::COLOR_CYAN,      // Foreground
                Driver::COLOR_DARKBLUE,  // Background
                Driver::COLOR_MAGENTA    // Accent
            ); 
            break;
        // ... other modes
    }
}
#endif
```

## Display Specifications

| Property | Value |
|----------|-------|
| Resolution | 128 x 160 pixels |
| Color Depth | 16-bit RGB565 (65K colors) |
| Interface | SPI (4-wire) |
| Controller | ST7735S |
| Refresh Rate | ~30 FPS |

## Troubleshooting

### Display is blank
- Check wiring connections
- Verify RST pin is connected to D30
- Check backlight connection (BLK to 3V3)

### Wrong colors
- Ensure ST7735S variant (not ST7735R)
- Check MADCTL setting in driver if colors are inverted

### Encoder not working
- Verify encoder pins: A=D12, B=D11, Click=D0
- Check if D0 conflicts with other hardware

## Author

Created by **noisehoho** for the Daisy/Oopsy community.

## License

MIT License - Same as Oopsy
