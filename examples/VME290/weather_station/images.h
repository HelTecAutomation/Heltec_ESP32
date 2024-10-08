#include <sys/types.h>
#define WiFi_Logo_width 60
#define WiFi_Logo_height 36
const uint8_t WiFi_Logo_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF,
  0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
  0xFF, 0x03, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0xFF, 0xFF, 0xFF, 0x07, 0xC0, 0x83, 0x01, 0x80, 0xFF, 0xFF, 0xFF,
  0x01, 0x00, 0x07, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0x00,
  0xC0, 0xFF, 0xFF, 0x7C, 0x00, 0x60, 0x0C, 0x00, 0xC0, 0x31, 0x46, 0x7C,
  0xFC, 0x77, 0x08, 0x00, 0xE0, 0x23, 0xC6, 0x3C, 0xFC, 0x67, 0x18, 0x00,
  0xE0, 0x23, 0xE4, 0x3F, 0x1C, 0x00, 0x18, 0x00, 0xE0, 0x23, 0x60, 0x3C,
  0x1C, 0x70, 0x18, 0x00, 0xE0, 0x03, 0x60, 0x3C, 0x1C, 0x70, 0x18, 0x00,
  0xE0, 0x07, 0x60, 0x3C, 0xFC, 0x73, 0x18, 0x00, 0xE0, 0x87, 0x70, 0x3C,
  0xFC, 0x73, 0x18, 0x00, 0xE0, 0x87, 0x70, 0x3C, 0x1C, 0x70, 0x18, 0x00,
  0xE0, 0x87, 0x70, 0x3C, 0x1C, 0x70, 0x18, 0x00, 0xE0, 0x8F, 0x71, 0x3C,
  0x1C, 0x70, 0x18, 0x00, 0xC0, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x08, 0x00,
  0xC0, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x0C, 0x00, 0x80, 0xFF, 0xFF, 0x1F,
  0x00, 0x00, 0x06, 0x00, 0x80, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x07, 0x00,
  0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0xF8, 0xFF, 0xFF,
  0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x00,
  0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF,
  0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
#define sunny_width 30
#define sunny_height 30
const uint8_t mini_sunny3030[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x00, 0xC0, 0x01, 0x00, 
  0x00, 0xC0, 0x01, 0x00, 0x80, 0xC3, 0xE1, 0x00, 0x80, 0xC7, 0xF0, 0x00, 
  0x00, 0x8F, 0x79, 0x00, 0x00, 0xF6, 0x37, 0x00, 0x00, 0xF8, 0x0F, 0x00, 
  0x00, 0x3C, 0x1E, 0x00, 0x00, 0x1C, 0x1C, 0x00, 0xF0, 0x1D, 0xDC, 0x07, 
  0xF0, 0x1D, 0xDC, 0x07, 0xA0, 0x1C, 0x9C, 0x01, 0x00, 0x3C, 0x1E, 0x00, 
  0x00, 0xF8, 0x0F, 0x00, 0x00, 0xF6, 0x37, 0x00, 0x00, 0xEF, 0x73, 0x00, 
  0x80, 0x87, 0xF0, 0x00, 0x80, 0xC3, 0xE1, 0x00, 0x00, 0xC1, 0x41, 0x00, 
  0x00, 0xC0, 0x01, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  };


// #define wi-day-sunny-1_width 64
// #define wi-day-sunny-1_height 64
const uint8_t  sunny6464[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x07, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x07, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x07, 0xC0, 0x07, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x80, 
  0x03, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0xF0, 0x01, 0x00, 
  0x00, 0x00, 0x7F, 0x00, 0x00, 0xF8, 0x01, 0x00, 0x00, 0x00, 0x7E, 0x00, 
  0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x80, 0x07, 0x7C, 0x00, 0x00, 
  0x00, 0x00, 0x38, 0xF8, 0x3F, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 
  0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1F, 
  0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x0F, 0xC0, 0x07, 0x00, 0x00, 
  0x00, 0x00, 0xC0, 0x07, 0xC0, 0x07, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 
  0x80, 0x07, 0x00, 0x00, 0x00, 0x60, 0xE0, 0x03, 0x80, 0x0F, 0x16, 0x00, 
  0x00, 0xFC, 0xE3, 0x01, 0x00, 0x8F, 0x7F, 0x00, 0x00, 0xFC, 0xE3, 0x01, 
  0x00, 0x8F, 0x7F, 0x00, 0x00, 0xFC, 0xE3, 0x01, 0x00, 0x8F, 0x7F, 0x00, 
  0x00, 0xFC, 0xE1, 0x03, 0x80, 0x0F, 0x3F, 0x00, 0x00, 0x00, 0xE0, 0x03, 
  0x80, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 0x80, 0x07, 0x00, 0x00, 
  0x00, 0x00, 0xC0, 0x07, 0xC0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0F, 
  0xE0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0xF8, 0x03, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xF8, 0x3F, 0x18, 0x00, 0x00, 
  0x00, 0x00, 0x78, 0xE0, 0x0F, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 
  0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0xFC, 0x00, 0x00, 
  0x00, 0x00, 0x3F, 0x00, 0x00, 0xF8, 0x01, 0x00, 0x00, 0x00, 0x1F, 0x80, 
  0x03, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x03, 0xE0, 0x01, 0x00, 
  0x00, 0x00, 0x02, 0xC0, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const uint8_t cloud1616minicloudy3030[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x78, 0x00, 0x00, 0xC0, 0xFE, 0x01, 0x00, 0xF8, 0xFF, 0x03, 
  0x00, 0xFE, 0x8F, 0x03, 0x00, 0x9E, 0x8F, 0x07, 0x00, 0x0F, 0x1E, 0x07, 
  0x80, 0x07, 0x7C, 0x07, 0xC0, 0x07, 0xFC, 0x07, 0xE0, 0x07, 0xFC, 0x03, 
  0xF0, 0x00, 0xC0, 0x03, 0x70, 0x00, 0x80, 0x03, 0x70, 0x00, 0x80, 0x03, 
  0x70, 0x00, 0x80, 0x03, 0xE0, 0x00, 0xC0, 0x03, 0xE0, 0xFF, 0xFF, 0x01, 
  0xC0, 0xFF, 0xFF, 0x00, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  };
const uint8_t minirain3030[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xF8, 0x07, 0x00, 
  0x00, 0xFC, 0x0F, 0x00, 0x00, 0x3E, 0x0F, 0x00, 0x00, 0x0E, 0x1E, 0x00, 
  0x00, 0x07, 0xFC, 0x00, 0xC0, 0x07, 0xFC, 0x01, 0xE0, 0x07, 0xF8, 0x03, 
  0xE0, 0x00, 0x80, 0x03, 0xE0, 0x00, 0x80, 0x03, 0x70, 0xB0, 0x1B, 0x07, 
  0xE0, 0xF8, 0x9F, 0x03, 0xE0, 0xB8, 0x9F, 0x03, 0xE0, 0xFF, 0xFD, 0x01, 
  0xC0, 0xFF, 0xFF, 0x01, 0x00, 0xDF, 0x7F, 0x00, 0x00, 0xFC, 0x0F, 0x00, 
  0x00, 0xFC, 0x0E, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 
  0x00, 0xE0, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  };



const uint8_t cloudy6464[]  = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0xF8, 
  0xE3, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x1F, 0x00, 
  0x00, 0x00, 0x80, 0xFF, 0xFF, 0x83, 0x3F, 0x00, 0x00, 0x00, 0xE0, 0xFF, 
  0xFF, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xF0, 0x2F, 0xFE, 0x00, 0x7C, 0x00, 
  0x00, 0x00, 0xF0, 0x03, 0xF8, 0x01, 0x7C, 0x00, 0x00, 0x00, 0xF8, 0x00, 
  0xF0, 0x01, 0x78, 0x00, 0x00, 0x00, 0x7C, 0x00, 0xE0, 0x03, 0xF8, 0x00, 
  0x00, 0x00, 0x7C, 0x00, 0xC0, 0x03, 0x78, 0x00, 0x00, 0x00, 0x3C, 0x00, 
  0xC0, 0x07, 0xF0, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x80, 0x7F, 0xF8, 0x00, 
  0x00, 0xC0, 0x3F, 0x00, 0x80, 0xFF, 0x79, 0x00, 0x00, 0xE0, 0x1F, 0x00, 
  0x80, 0xFF, 0x7B, 0x00, 0x00, 0xF0, 0x3F, 0x00, 0x80, 0xFF, 0x7F, 0x00, 
  0x00, 0xF8, 0x03, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0xF8, 0x00, 0x00, 
  0x00, 0x80, 0x3F, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 
  0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x3E, 0x00, 0x00, 
  0x00, 0x00, 0x3E, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 
  0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x3E, 0x00, 0x00, 
  0x00, 0x00, 0x3C, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 
  0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x7C, 0x00, 0x00, 
  0x00, 0x00, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 
  0x00, 0xF8, 0x01, 0x00, 0x00, 0xC0, 0x0F, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x0F, 0x00, 0x00, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00, 
  0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x48, 0x52, 0x22, 0x02, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };


const uint8_t rain6464_r0ta90[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xF8, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xFC, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x0E, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x1C, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0C, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFC, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x7F, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x7E, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0xC0, 0x01, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x7F, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x7F, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0xE0, 0x01, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x1F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x7C, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x4C, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x1C, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x0E, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xFC, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xF8, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const uint8_t rain646490[] = {
  0x00, 0x80, 0xFF, 0xFE, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x80, 0x1F, 0xF8, 
  0xFF, 0x1F, 0x00, 0x00, 0x00, 0xC0, 0x0F, 0xF8, 0xC3, 0x3F, 0x00, 0x00, 
  0x00, 0xC0, 0x07, 0xFC, 0x00, 0x3F, 0x00, 0x00, 0x00, 0xE0, 0x07, 0x7C, 
  0x00, 0x3E, 0x00, 0x00, 0x00, 0xE0, 0x03, 0x7E, 0x00, 0x7C, 0x00, 0x00, 
  0x00, 0xE0, 0x03, 0x3E, 0x00, 0x7C, 0x00, 0x00, 0x00, 0xE0, 0x03, 0x3E, 
  0x00, 0x7C, 0x00, 0x00, 0x00, 0xE0, 0x03, 0x3E, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0xE0, 0x83, 0x3F, 0x00, 0x78, 0x00, 0x00, 0x00, 0xE0, 0xE7, 0x3F, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0xC0, 0xF7, 0x3F, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0xC0, 0xFF, 0x3F, 0x00, 0x78, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x03, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x00, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0x00, 0x7F, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0x00, 0x1F, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x80, 0x0F, 0x00, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x80, 0x0F, 0x00, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0x80, 0x0F, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x80, 0x0F, 0x00, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0x00, 0x1F, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0x00, 0x7E, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x03, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0x00, 0xF8, 0x3F, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x3F, 0x00, 0x7C, 0x00, 0x00, 
  0x00, 0x00, 0x80, 0x3F, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 
  0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x3E, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xFC, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 
  0xC3, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0x1F, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xF0, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 
  0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const uint8_t rain6464xbm[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x1F, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x80, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x3F, 0xFE, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0xF0, 0x07, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x01, 
  0xE0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0xC0, 0x07, 0x00, 0x00, 
  0x00, 0x00, 0xF8, 0x00, 0xC0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 
  0x80, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x80, 0xFF, 0x00, 0x00, 
  0x00, 0x80, 0x3F, 0x00, 0x00, 0xFF, 0x03, 0x00, 0x00, 0xC0, 0x3F, 0x00, 
  0x00, 0xFF, 0x07, 0x00, 0x00, 0xE0, 0x3F, 0x00, 0x00, 0xFF, 0x0F, 0x00, 
  0x00, 0xF0, 0x03, 0x00, 0x00, 0x80, 0x1F, 0x00, 0x00, 0xF8, 0x01, 0x00, 
  0x00, 0x00, 0x3F, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x7C, 0x00, 0x00, 
  0x00, 0x00, 0x7C, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 
  0x00, 0x3C, 0x00, 0x00, 0x04, 0x02, 0x78, 0x00, 0x00, 0x3C, 0x00, 0x1E, 
  0x0E, 0x07, 0x78, 0x00, 0x00, 0x7C, 0x00, 0x1E, 0x8F, 0x0F, 0x7C, 0x00, 
  0x00, 0x7C, 0x00, 0x1E, 0x8F, 0x07, 0x7C, 0x00, 0x00, 0xF8, 0x00, 0x1F, 
  0x8F, 0x07, 0x3E, 0x00, 0x00, 0xF8, 0x00, 0x9F, 0xCF, 0x07, 0x3E, 0x00, 
  0x00, 0xF0, 0x03, 0x8F, 0xC7, 0x87, 0x1F, 0x00, 0x00, 0xE0, 0x9F, 0x8F, 
  0xC7, 0xF3, 0x1F, 0x00, 0x00, 0xE0, 0x9F, 0xCF, 0xC7, 0xF3, 0x0F, 0x00, 
  0x00, 0x80, 0x9F, 0xC7, 0xE7, 0xFB, 0x03, 0x00, 0x00, 0x00, 0x9F, 0xC7, 
  0xE3, 0xF1, 0x01, 0x00, 0x00, 0x00, 0xC8, 0xC7, 0xE3, 0x21, 0x00, 0x00, 
  0x00, 0x00, 0xC0, 0xE7, 0xF3, 0x01, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE3, 
  0xF3, 0x01, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE3, 0xF1, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xC0, 0xE3, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF1, 
  0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

const uint8_t rain6464[]  = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x1F, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x80, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x3F, 0xFE, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0xF0, 0x07, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x01, 
  0xE0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0xC0, 0x07, 0x00, 0x00, 
  0x00, 0x00, 0xF8, 0x00, 0xC0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 
  0x80, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x80, 0xFF, 0x00, 0x00, 
  0x00, 0x80, 0x3F, 0x00, 0x00, 0xFF, 0x03, 0x00, 0x00, 0xC0, 0x3F, 0x00, 
  0x00, 0xFF, 0x07, 0x00, 0x00, 0xE0, 0x3F, 0x00, 0x00, 0xFF, 0x0F, 0x00, 
  0x00, 0xF0, 0x03, 0x00, 0x00, 0x80, 0x1F, 0x00, 0x00, 0xF8, 0x01, 0x00, 
  0x00, 0x00, 0x3F, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x7C, 0x00, 0x00, 
  0x00, 0x00, 0x7C, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 
  0x00, 0x3C, 0x00, 0x00, 0x04, 0x02, 0x78, 0x00, 0x00, 0x3C, 0x00, 0x1E, 
  0x0E, 0x07, 0x78, 0x00, 0x00, 0x7C, 0x00, 0x1E, 0x8F, 0x0F, 0x7C, 0x00, 
  0x00, 0x7C, 0x00, 0x1E, 0x8F, 0x07, 0x7C, 0x00, 0x00, 0xF8, 0x00, 0x1F, 
  0x8F, 0x07, 0x3E, 0x00, 0x00, 0xF8, 0x00, 0x9F, 0xCF, 0x07, 0x3E, 0x00, 
  0x00, 0xF0, 0x03, 0x8F, 0xC7, 0x87, 0x1F, 0x00, 0x00, 0xE0, 0x9F, 0x8F, 
  0xC7, 0xF3, 0x1F, 0x00, 0x00, 0xE0, 0x9F, 0xCF, 0xC7, 0xF3, 0x0F, 0x00, 
  0x00, 0x80, 0x9F, 0xC7, 0xE7, 0xFB, 0x03, 0x00, 0x00, 0x00, 0x9F, 0xC7, 
  0xE3, 0xF1, 0x01, 0x00, 0x00, 0x00, 0xC8, 0xC7, 0xE3, 0x21, 0x00, 0x00, 
  0x00, 0x00, 0xC0, 0xE7, 0xF3, 0x01, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE3, 
  0xF3, 0x01, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE3, 0xF1, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xC0, 0xE3, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF1, 
  0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const uint8_t  wifi_bitfis[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x0F, 0xFC, 0x1F, 0x0E, 0x70, 
  0xE2, 0x67, 0xF8, 0x1E, 0x18, 0x18, 0xE0, 0x07, 0x60, 0x06, 0x80, 0x00, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00,};
const uint8_t  wifix_bitfis[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1F, 0x00, 
  0xF0, 0xFF, 0x00, 0xFC, 0xFF, 0x03, 0x3E, 0xC0, 0x07, 0x8E, 0x1F, 0x07, 
  0xF0, 0xFF, 0x00, 0xF8, 0xF0, 0x01, 0x38, 0xC0, 0x01, 0x80, 0x1F, 0x00, 
  0xC0, 0x3F, 0x00, 0xC0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 
  0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const uint8_t wifi_off[] =
 {
  0x00, 0x00, 0x04, 0x00, 0xEC, 0x03, 0xFE, 0x0F, 0x3F, 0x1E, 0xFA, 0x0B, 
  0xFC, 0x07, 0xF0, 0x01, 0xF0, 0x03, 0x40, 0x07, 0xE0, 0x06, 0x00, 0x04, 
  0x00, 0x00, };

const uint8_t  wind_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xC0, 0x0F, 0x00, 0x00, 0x00, 
  0xF0, 0x0F, 0x00, 0x00, 0x00, 0xFC, 0x07, 0x00, 0x00, 0x00, 0xFF, 0x03, 
  0x00, 0x00, 0xE0, 0xFF, 0x03, 0x00, 0x00, 0xF8, 0xFF, 0x01, 0x00, 0x00, 
  0xFE, 0xFF, 0x01, 0x00, 0x80, 0xFF, 0xFF, 0x00, 0x00, 0xF0, 0xFF, 0x7F, 
  0x00, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0x00, 0xFF, 0xFF, 0x3F, 0x00, 0xC0, 
  0xFF, 0xFF, 0x3F, 0x00, 0xF0, 0xFF, 0xFF, 0x3F, 0x00, 0xF0, 0xFF, 0xFF, 
  0x7F, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x01, 
  0x00, 0xFC, 0xFF, 0xFF, 0x03, 0x00, 0xC0, 0xFF, 0xFF, 0x07, 0x00, 0x00, 
  0xFC, 0xFF, 0x07, 0x00, 0x00, 0xC0, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0xFC, 
  0x1F, 0x00, 0x00, 0x00, 0xC0, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const uint8_t  wifi2_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xF0, 0x01, 0xF0, 0x01, 0x40, 0x00, 0xE0, 0x00, 0x00, 0x00, 
  0x00, 0x00, };
const uint8_t  wifi1_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, };
#define battery_w 13
#define battery_h 13
const uint8_t batteryfull[] = {
  0x00, 0x00, 0xE0, 0x00, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0x00, 0x00, };
const uint8_t battery6[] = {
  0x00, 0x00, 0xE0, 0x00, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0x00, 0x00, };
const uint8_t battery5[] = {
  0x00, 0x00, 0xE0, 0x00, 0xF8, 0x03, 0xF8, 0x03, 0x18, 0x03, 0xF8, 0x03, 
  0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0x00, 0x00, };
const uint8_t battery4[] = {
  0x00, 0x00, 0xE0, 0x00, 0xF8, 0x03, 0xF8, 0x03, 0x18, 0x03, 0x18, 0x03, 
  0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0x00, 0x00, };
 const uint8_t battery3[] = {
  0x00, 0x00, 0xE0, 0x00, 0xF8, 0x03, 0xF8, 0x03, 0x18, 0x03, 0x18, 0x03, 
  0x18, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0x00, 0x00, };
const uint8_t battery2[] = {
  0x00, 0x00, 0xE0, 0x00, 0xF8, 0x03, 0xF8, 0x03, 0x18, 0x03, 0x18, 0x03, 
  0x18, 0x03, 0x18, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0x00, 0x00, };
const uint8_t battery1[] = {
  0x00, 0x00, 0xE0, 0x00, 0xF8, 0x03, 0xF8, 0x03, 0x18, 0x03, 0x18, 0x03, 
  0x18, 0x03, 0x18, 0x03, 0x18, 0x03, 0xF8, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0x00, 0x00, };
const uint8_t battery0[] = {
  0x00, 0x00, 0xE0, 0x00, 0xF8, 0x03, 0xF8, 0x03, 0x18, 0x03, 0x18, 0x03, 
  0x18, 0x03, 0x18, 0x03, 0x18, 0x03, 0x18, 0x03, 0xF8, 0x03, 0xF8, 0x03, 
  0x00, 0x00, };
const uint8_t hum[] = {
 0x20, 0x00, 0x70, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0xFC, 0x01, 0xFC, 0x01, 
  0xFE, 0x03, 0xFE, 0x03, 0xFF, 0x07, 0xFF, 0x07, 0xFF, 0x07, 0xFF, 0x07, 
  0xFF, 0x07, 0xFF, 0x07, 0xFE, 0x03, 0xFC, 0x01,  };
const uint8_t temp[] = {
  0x78, 0x00, 0xFC, 0x00, 0xCC, 0x00, 0xCC, 0x00, 0xCC, 0x00, 0xCC, 0x00, 
  0xCC, 0x00, 0xFC, 0x00, 0xFE, 0x01, 0xB7, 0x03, 0x33, 0x03, 0x7B, 0x03, 
  0x7B, 0x03, 0x03, 0x03, 0x87, 0x03, 0xFE, 0x01, 0xFC, 0x00,   };
  const uint8_t rain1616[] = {
  0x00, 0xFC, 0x03, 0x00, 0x00, 0xFF, 0x0F, 0x00, 0x00, 0xFF, 0x0F, 0x00, 
  0x80, 0x07, 0x3F, 0x00, 0xC0, 0x03, 0x3C, 0x00, 0xC0, 0x01, 0x38, 0x00, 
  0xE0, 0x01, 0xF8, 0x01, 0xF8, 0x00, 0xF8, 0x07, 0xFC, 0x00, 0xF0, 0x0F, 
  0x7E, 0x00, 0xF0, 0x1F, 0x0F, 0x00, 0x00, 0x3C, 0x0F, 0x00, 0x00, 0x3C, 
  0x07, 0x00, 0x00, 0x38, 0x07, 0x88, 0x31, 0x38, 0x07, 0x9C, 0x7B, 0x38, 
  0x07, 0xDE, 0x7B, 0x38, 0x0F, 0xDE, 0x3B, 0x3C, 0x1E, 0xCE, 0x39, 0x1E, 
  0xFE, 0xCE, 0xFD, 0x1F, 0xFC, 0xEF, 0xFD, 0x0F, 0xF0, 0xEF, 0xDD, 0x03, 
  0x00, 0xE7, 0x1C, 0x00, 0x80, 0xF7, 0x1E, 0x00, 0x80, 0xF7, 0x0E, 0x00, 
  0x00, 0x73, 0x0E, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0x78, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, };
const uint8_t cloud1616[] = {
  0x00, 0x00, 0xF8, 0x03, 0x00, 0xF8, 0xFC, 0x0F, 0x00, 0xFE, 0xFF, 0x1F, 
  0x00, 0xFF, 0x1F, 0x1E, 0x80, 0xDF, 0x0F, 0x3C, 0xC0, 0x03, 0x1E, 0x38, 
  0xE0, 0x01, 0x3C, 0x38, 0xE0, 0x01, 0x3C, 0x38, 0xF8, 0x00, 0xF8, 0x39, 
  0xFC, 0x00, 0xF8, 0x3F, 0xFE, 0x00, 0xF8, 0x3F, 0x1F, 0x00, 0x00, 0x1F, 
  0x0F, 0x00, 0x00, 0x1E, 0x07, 0x00, 0x00, 0x1C, 0x07, 0x00, 0x00, 0x3C, 
  0x07, 0x00, 0x00, 0x1C, 0x07, 0x00, 0x00, 0x1C, 0x0F, 0x00, 0x00, 0x1E, 
  0x1F, 0x00, 0x00, 0x1F, 0xFE, 0xFF, 0xFF, 0x0F, 0xFC, 0xFF, 0xFF, 0x07, 
  0xF8, 0xFF, 0xFF, 0x03,  };
const uint8_t sun1616[] = {
 0x00, 0xE0, 0x01, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0xE0, 0x01, 0x00, 
  0x00, 0xE0, 0x01, 0x00, 0x70, 0xE0, 0x81, 0x03, 0xF0, 0xC0, 0xC0, 0x03, 
  0xF0, 0x01, 0xE0, 0x03, 0xE0, 0xE1, 0xE1, 0x01, 0xC0, 0xF8, 0xC7, 0x00, 
  0x00, 0xFC, 0x0F, 0x00, 0x00, 0xFE, 0x1F, 0x00, 0x00, 0x0F, 0x3C, 0x00, 
  0x00, 0x07, 0x38, 0x00, 0x9F, 0x07, 0x78, 0x3E, 0xBF, 0x07, 0x78, 0x3F, 
  0xBF, 0x07, 0x78, 0x3F, 0x9F, 0x07, 0x78, 0x3E, 0x00, 0x07, 0x38, 0x00, 
  0x00, 0x0F, 0x3C, 0x00, 0x00, 0xFE, 0x1F, 0x00, 0x00, 0xFC, 0x0F, 0x00, 
  0xC0, 0xF8, 0xC7, 0x00, 0xE0, 0xE1, 0xE1, 0x01, 0xF0, 0x01, 0xE0, 0x03, 
  0xF0, 0xC0, 0xC0, 0x03, 0x70, 0xE0, 0x81, 0x03, 0x00, 0xE0, 0x01, 0x00, 
  0x00, 0xE0, 0x01, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0xE0, 0x01, 0x00, };