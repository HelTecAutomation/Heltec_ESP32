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

#include "HT_st7789spi.h"
#include <limits.h>
#include <SPI.h>

#define SPI_DEFAULT_FREQ 32000000  ///< Default SPI data clock frequency

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST7789 driver with software SPI
    @param  w     Display width in pixels at default rotation setting (0)
    @param  h     Display height in pixels at default rotation setting (0)
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  mosi  SPI MOSI pin #
    @param  sclk  SPI Clock pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
    @param  miso  SPI MISO pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
HT_ST7789::HT_ST7789(uint16_t w, uint16_t h, int8_t cs, int8_t dc,
                     int8_t mosi, int8_t sclk, int8_t rst,
                     int8_t miso)
  : Adafruit_SPITFT(w, h, cs, dc, mosi, sclk, rst, miso) {}

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST7789 driver with hardware SPI
    @param  w     Display width in pixels at default rotation setting (0)
    @param  h     Display height in pixels at default rotation setting (0)
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
HT_ST7789::HT_ST7789(uint16_t w, uint16_t h, int8_t cs, int8_t dc,
                     int8_t rst)
  : Adafruit_SPITFT(w, h, cs, dc, rst) {}

#if !defined(ESP8266)
/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST7789 driver with selectable hardware SPI
    @param  w     Display width in pixels at default rotation setting (0)
    @param  h     Display height in pixels at default rotation setting (0)
    @param  spiClass A pointer to an SPI device to use (e.g. &SPI1)
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
HT_ST7789::HT_ST7789(uint16_t w, uint16_t h, SPIClass *spiClass,
                     int8_t cs, int8_t dc, int8_t rst)
  : Adafruit_SPITFT(w, h, spiClass, cs, dc, rst) {




    
  }
#endif  // end !ESP8266

/**************************************************************************/
/*!
    @brief  Companion code to the initiliazation tables. Reads and issues
            a series of LCD commands stored in PROGMEM byte array.
    @param  addr  Flash memory array with commands and data to send
*/
/**************************************************************************/
void HT_ST7789::displayInit(const uint8_t *addr) {

  uint8_t numCommands, cmd, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);  // Number of commands to follow
  while (numCommands--) {               // For each command...
    cmd = pgm_read_byte(addr++);        // Read command
    numArgs = pgm_read_byte(addr++);    // Number of args to follow
    ms = numArgs & ST_CMD_DELAY;        // If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;           // Mask out delay bit
    sendCommand(cmd, addr, numArgs);
    addr += numArgs;

    if (ms) {
      ms = pgm_read_byte(addr++);  // Read post-command delay time (ms)
      if (ms == 255)
        ms = 500;  // If 255, delay for 500 ms
      delay(ms);
    }
  }
}

/**************************************************************************/
/*!
    @brief  Initialize ST7789 chip. Connects to the ST7789 over SPI and
            sends initialization procedure commands
    @param  freq  Desired SPI clock frequency
*/
/**************************************************************************/
void HT_ST7789::begin(uint32_t freq) {
  if (!freq) {
    freq = SPI_DEFAULT_FREQ;
  }
  _freq = freq;

  invertOnCommand = ST7789_INVON;
  invertOffCommand = ST7789_INVOFF;

  initSPI(freq, spiMode);
}

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7789 displays
    @param  cmdList  Flash memory array with commands and data to send
*/
/**************************************************************************/
void HT_ST7789::commonInit(const uint8_t *cmdList) {
  begin();

  if (cmdList) {
    displayInit(cmdList);
  }
}
static const uint8_t PROGMEM
  generic_st7789[] =  {                // Init commands for 7789 screens
    9,                              //  9 commands in list:
    ST7789_SWRESET,   ST_CMD_DELAY, //  1: Software reset, no args, w/delay
      150,                          //     ~150 ms delay
    ST7789_SLPOUT ,   ST_CMD_DELAY, //  2: Out of sleep mode, no args, w/delay
      10,                          //      10 ms delay
    ST7789_COLMOD , 1+ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
      0x55,                         //     16-bit color
      10,                           //     10 ms delay
    ST7789_MADCTL , 1,              //  4: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    ST7789_CASET  , 4,              //  5: Column addr set, 4 args, no delay:
      0x00,
      0,        //     XSTART = 0
      0,
      240,  //     XEND = 240
    ST7789_RASET  , 4,              //  6: Row addr set, 4 args, no delay:
      0x00,
      0,             //     YSTART = 0
      320>>8,
      320&0xFF,  //     YEND = 320
    ST7789_INVON  ,   ST_CMD_DELAY,  //  7: hack
      10,
    ST7789_NORON  ,   ST_CMD_DELAY, //  8: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST7789_DISPON ,   ST_CMD_DELAY, //  9: Main screen turn on, no args, delay
      10 };                          //    10 ms delay
// clang-format on

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7789 displays
    @param  width  Display width
    @param  height Display height
    @param  mode   SPI data mode; one of SPI_MODE0, SPI_MODE1, SPI_MODE2
                   or SPI_MODE3 (do NOT pass the numbers 0,1,2 or 3 -- use
                   the defines only, the values are NOT the same!)
*/
/**************************************************************************/
void HT_ST7789::init(uint16_t width, uint16_t height, uint8_t mode) {

  spiMode = mode;

  commonInit(NULL);
  if (width == 240 && height == 240) {
    // 1.3", 1.54" displays (right justified)
    _rowstart = (320 - height);
    _rowstart2 = 0;
    _colstart = _colstart2 = (240 - width);
  } else if (width == 135 && height == 240) {
    // 1.14" display (centered, with odd size)
    _rowstart = _rowstart2 = (int)((320 - height) / 2);
    // This is the only device currently supported device that has different
    // values for _colstart & _colstart2. You must ensure that the extra
    // pixel lands in _colstart and not in _colstart2
    _colstart = (int)((240 - width + 1) / 2);
    _colstart2 = (int)((240 - width) / 2);
  } else {
    // 1.47", 1.69, 1.9", 2.0" displays (centered)
    _rowstart = _rowstart2 = (int)((320 - height) / 2);
    _colstart = _colstart2 = (int)((240 - width) / 2);
  }

  windowWidth = width;
  windowHeight = height;

  displayInit(generic_st7789);
}



/**************************************************************************/
/*!
  @brief  SPI displays set an address window rectangle for blitting pixels
  @param  x  Top left corner x coordinate
  @param  y  Top left corner x coordinate
  @param  w  Width of window
  @param  h  Height of window
*/
/**************************************************************************/
void HT_ST7789::setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
                              uint16_t h) {
x += _xstart;
  y += _ystart;

  uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
  uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

  writeCommand(ST7789_CASET);  // Column addr set
  // SPI_WRITE16(0);
  SPI_WRITE32(xa);
  writeCommand(ST7789_RASET);  // Row addr set
  // SPI_WRITE16(0);
  SPI_WRITE32(ya);

  writeCommand(ST7789_RAMWR);  // write to RAM
}

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void HT_ST7789::setRotation(uint8_t m) {
uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  switch (rotation) {
  case 0:
    madctl = ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB;
    _xstart = _colstart;
    _ystart = _rowstart;
    _width = windowWidth;
    _height = windowHeight;
    break;
  case 1:
    madctl = ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
    _xstart = _rowstart;
    _ystart = _colstart2;
    _height = windowWidth;
    _width = windowHeight;
    break;
  case 2:
    madctl = ST7789_MADCTL_RGB;
    _xstart = _colstart2;
    _ystart = _rowstart2;
    _width = windowWidth;
    _height = windowHeight;
    break;
  case 3:
    madctl = ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB;
    _xstart = _rowstart2;
    _ystart = _colstart;
    _height = windowWidth;
    _width = windowHeight;
    break;
  }

  sendCommand(ST7789_MADCTL, &madctl, 1);
}

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) of display with offsets
    @param  col  The offset from 0 for the column address
    @param  row  The offset from 0 for the row address
*/
/**************************************************************************/
void HT_ST7789::setColRowStart(int8_t col, int8_t row) {
  _colstart = col;
  _rowstart = row;
}

/**************************************************************************/
/*!
 @brief  Change whether display is on or off
 @param  enable True if you want the display ON, false OFF
 */
/**************************************************************************/
void HT_ST7789::enableDisplay(boolean enable) {
  sendCommand(enable ? ST7789_DISPON : ST7789_DISPOFF);
}

/**************************************************************************/
/*!
 @brief  Change whether TE pin output is on or off
 @param  enable True if you want the TE pin ON, false OFF
 */
/**************************************************************************/
void HT_ST7789::enableTearing(boolean enable) {
  sendCommand(enable ? ST7789_TEON : ST7789_TEOFF);
}

/**************************************************************************/
/*!
 @brief  Change whether sleep mode is on or off
 @param  enable True if you want sleep mode ON, false OFF
 */
/**************************************************************************/
void HT_ST7789::enableSleep(boolean enable) {
  sendCommand(enable ? ST7789_SLPIN : ST7789_SLPOUT);
}
 /**************************************************************************/
 /*!
 * @brief Vertical Scrolling Definition.
 * @param   tfa    top fixed area
 * @param   vsa    scroll area
 * @param   bta    bottom fixed area
 * @return  errcode
 * @retval  0      success
 * @retval  -1     fail 
 /*!
 /***************************************************************************/
int HT_ST7789::LCD_Set_Scroll_Area(uint16_t tfa, uint16_t vsa, uint16_t bta)
{
    uint16_t data;
    
    if (tfa + vsa + bta != 320) {
        return -1;
    }
    
    sendCommand(0x33);
 
    SPI_WRITE16(tfa);
    SPI_WRITE16(vsa);
    SPI_WRITE16(bta);
    
    return 0;
}
/**
 * @brief Set Vertical scroll start address of RAM.
 * @param   vsp    scroll start address of RAM
 * @return  none
 */
void HT_ST7789::LCD_Set_Scroll_Start_Address(uint16_t vsp)
{
    
    sendCommand(0x37);
    
    spiWrite(vsp / 256);
    spiWrite(vsp % 256);
}





////////// stuff not actively being used, but kept for posterity
/*

 uint8_t HT_ST7789::spiread(void) {
 uint8_t r = 0;
 if (_sid > 0) {
 r = shiftIn(_sid, _sclk, MSBFIRST);
 } else {
 //SID_DDR &= ~_BV(SID);
 //int8_t i;
 //for (i=7; i>=0; i--) {
 //  SCLK_PORT &= ~_BV(SCLK);
 //  r <<= 1;
 //  r |= (SID_PIN >> SID) & 0x1;
 //  SCLK_PORT |= _BV(SCLK);
 //}
 //SID_DDR |= _BV(SID);

 }
 return r;
 }

 void HT_ST7789::dummyclock(void) {

 if (_sid > 0) {
 digitalWrite(_sclk, LOW);
 digitalWrite(_sclk, HIGH);
 } else {
 // SCLK_PORT &= ~_BV(SCLK);
 //SCLK_PORT |= _BV(SCLK);
 }
 }
 uint8_t HT_ST7789::readdata(void) {
 *portOutputRegister(rsport) |= rspin;

 *portOutputRegister(csport) &= ~ cspin;

 uint8_t r = spiread();

 *portOutputRegister(csport) |= cspin;

 return r;

 }

 uint8_t HT_ST7789::readcommand8(uint8_t c) {
 digitalWrite(_rs, LOW);

 *portOutputRegister(csport) &= ~ cspin;

 spiwrite(c);

 digitalWrite(_rs, HIGH);
 pinMode(_sid, INPUT); // input!
 digitalWrite(_sid, LOW); // low
 spiread();
 uint8_t r = spiread();


 *portOutputRegister(csport) |= cspin;


 pinMode(_sid, OUTPUT); // back to output
 return r;
 }


 uint16_t HT_ST7789::readcommand16(uint8_t c) {
 digitalWrite(_rs, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);

 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 uint16_t r = spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);

 pinMode(_sid, OUTPUT); // back to output
 return r;
 }

 uint32_t HT_ST7789::readcommand32(uint8_t c) {
 digitalWrite(_rs, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 spiwrite(c);
 pinMode(_sid, INPUT); // input!

 dummyclock();
 dummyclock();

 uint32_t r = spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);

 pinMode(_sid, OUTPUT); // back to output
 return r;
 }

 */
