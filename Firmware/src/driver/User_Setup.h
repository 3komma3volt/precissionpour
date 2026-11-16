#ifndef USER_SETUP_H
#define USER_SETUP_H

// User defined information reported by "Read_User_Setup" test & diagnostics example
#define USER_SETUP_INFO "ESP32_2432S028_Setup"

// ESP32-2432S028 pin assignments
#define TFT_MISO 12
#define TFT_MOSI 13  
#define TFT_SCLK 14
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST  -1  // Reset pin (set to -1 if not used)

// Backlight control
#define TFT_BL   21  // LED back-light control pin

// Touch screen chip select (for TFT_eSPI touch - optional since you use TFT_Touch)
#define TOUCH_CS 
33

// Font loading
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

// SPI frequency settings
#define SPI_FREQUENCY  55000000  // 55MHz for display
#define SPI_READ_FREQUENCY  20000000  // 20MHz for reading from display
#define SPI_TOUCH_FREQUENCY  2500000  // 2.5MHz for touch controller

#endif // USER_SETUP_H