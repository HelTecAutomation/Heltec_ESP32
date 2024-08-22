/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#ifndef _HT_ST7789SPI_H_
#define _HT_ST7789SPI_H_

#include "Arduino.h"
#include "Print.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>

#define ST_CMD_DELAY 0x80  // special signifier for command lists

#define ST7789_NOP 0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID 0x04
#define ST7789_RDDST 0x09

#define ST7789_SLPIN 0x10
#define ST7789_SLPOUT 0x11
#define ST7789_PTLON 0x12
#define ST7789_NORON 0x13

#define ST7789_INVOFF 0x20
#define ST7789_INVON 0x21
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_RAMRD 0x2E

#define ST7789_PTLAR 0x30
#define ST7789_TEOFF 0x34
#define ST7789_TEON 0x35
#define ST7789_MADCTL 0x36
#define ST7789_COLMOD 0x3A

#define ST7789_MADCTL_MY 0x80
#define ST7789_MADCTL_MX 0x40
#define ST7789_MADCTL_MV 0x20
#define ST7789_MADCTL_ML 0x10
#define ST7789_MADCTL_RGB 0x00

#define ST7789_RDID1 0xDA
#define ST7789_RDID2 0xDB
#define ST7789_RDID3 0xDC
#define ST7789_RDID4 0xDD

// Some ready-made 16-bit ('565') color settings:
#define ST7789_BLACK 0x0000
#define ST7789_WHITE 0xFFFF
#define ST7789_RED 0xF800
#define ST7789_GREEN 0x07E0
#define ST7789_BLUE 0x001F
#define ST7789_CYAN 0x07FF
#define ST7789_MAGENTA 0xF81F
#define ST7789_YELLOW 0xFFE0
#define ST7789_ORANGE 0xFC00

/// Subclass of SPITFT for ST7789 displays (lots in common!)
class HT_ST7789 : public Adafruit_SPITFT {
public:
  HT_ST7789(uint16_t w, uint16_t h, int8_t _CS, int8_t _DC, int8_t _MOSI,
            int8_t _SCLK, int8_t _RST = -1, int8_t _MISO = -1);
  HT_ST7789(uint16_t w, uint16_t h, int8_t CS, int8_t RS,
            int8_t RST = -1);
#if !defined(ESP8266)
  HT_ST7789(uint16_t w, uint16_t h, SPIClass *spiClass, int8_t CS,
            int8_t RS, int8_t RST = -1);
#endif  // end !ESP8266
  void init(uint16_t width, uint16_t height, uint8_t spiMode = SPI_MODE0);
  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void setRotation(uint8_t r);
  void enableDisplay(boolean enable);
  void enableTearing(boolean enable);
  void enableSleep(boolean enable);
  int  LCD_Set_Scroll_Area(uint16_t tfa, uint16_t vsa, uint16_t bta);
  void LCD_Set_Scroll_Start_Address(uint16_t vsp);
protected:
  uint16_t _colstart = 0,  ///< Some displays need this changed to offset
    _rowstart = 0,        ///< Some displays need this changed to offset
    _colstart2 = 0,       ///< Some displays need this changed to offset
    _rowstart2 = 0,       ///< Some displays need this changed to offset
    spiMode = SPI_MODE0;  ///< Certain display needs MODE3 instead
  uint16_t windowWidth;
  uint16_t windowHeight;
  void begin(uint32_t freq = 0);
  void commonInit(const uint8_t *cmdList);
  void displayInit(const uint8_t *addr);
  void setColRowStart(int8_t col, int8_t row);
  void sleep();
};

#endif  // _HT_ST7789H_
