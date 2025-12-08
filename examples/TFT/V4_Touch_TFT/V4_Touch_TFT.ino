#include "Arduino.h"
#include "HT_st7789spi.h"
#include "chsc6x.h"

#define TFT_CS          15
#define TFT_RST         18 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC          16
#define TFT_MOSI        33  // Data out
#define TFT_SCLK        17  // Clock out
#define TFT_BLK         21

#define TOUCH_SDA_PIN      47
#define TOUCH_SCL_PIN      48
#define TOUCH_INT_PIN      45
#define TOUCH_RST_PIN      44

#define Vext_Ctrl       36

#define PEN_THIN        1
#define PEN_MEDIUM      3
#define PEN_THICK       5

const unsigned char epd_bitmap_Bitmap [] PROGMEM = {
	// 'clean-24', 24x24px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 
	0xff, 0x98, 0x1f, 0xff, 0x98, 0x3f, 0xff, 0x98, 0x3f, 0xff, 0x98, 0x3f, 0xff, 0x98, 0x3f, 0xff, 
	0x9c, 0x3f, 0xff, 0x9c, 0x3f, 0xff, 0x9c, 0x3f, 0xff, 0x9c, 0x3f, 0xff, 0x98, 0x3f, 0xff, 0x98, 
	0x3f, 0xff, 0x98, 0x1f, 0xff, 0x98, 0x0f, 0xff, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
uint16_t  current_color = ST7789_BLACK;
uint16_t  touchX,touchY;
uint16_t lastX = 0, lastY = 0;
bool isDrawing = false;
uint8_t penSize = PEN_MEDIUM;

chsc6x touch(&Wire1,TOUCH_SDA_PIN,TOUCH_SCL_PIN,TOUCH_INT_PIN,TOUCH_RST_PIN);

HT_ST7789 tft(240,320,TFT_CS,TFT_DC,TFT_MOSI,TFT_SCLK,TFT_RST);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello! ST77xx TFT Test");
  VextON();
  delay(100);
  tft.init(240,320);
  touch.chsc6x_init();
  tft.invertDisplay(1);
  // delay(100);
  tft.fillScreen(ST7789_WHITE);
  blk_ctrl(1);
  draw_button_palette();
  Serial.println(F("Initialized"));
}

void loop() {
  // put your main code here, to run repeatedly:
  drawing_board();
  delay(50);
}

void draw_button_palette(void) {
  uint16_t colors[] = {
    ST7789_BLACK, ST7789_RED, ST7789_GREEN, ST7789_BLUE,
    ST7789_YELLOW, ST7789_MAGENTA, ST7789_CYAN
  };

  tft.drawRect(210,0,30,40,ST7789_BLACK);
  tft.drawBitmap(215,8,epd_bitmap_Bitmap,24,24,ST7789_BLACK);

  tft.drawRect(210, 40, 30, 30, ST7789_BLACK);
  tft.fillCircle(225, 55, penSize, ST7789_BLACK);

  for (int i = 0; i < 7; i++) {
    tft.fillRect(210, 70 + i * 30, 30, 30, colors[i]);
    tft.drawRect(210, 70 + i * 30, 30, 30, ST7789_BLACK);
  }
  choice_color(&current_color);
}

void choice_color(uint16_t *color) {
  printf("current_color: %4X \n",*color);
  switch(*color) {
    case ST7789_BLACK :
      *color = ST7789_BLACK;
      printf("black\n");
      break;
    case ST7789_RED :
      *color = ST7789_RED;
      printf("ST7789_RED\n");
      break;
    case ST7789_GREEN :
      *color = ST7789_GREEN;
      printf("ST7789_GREEN\n");
      break;
    case ST7789_BLUE :
      *color = ST7789_BLUE;
      printf("ST7789_BLUE\n");
      break;
    case ST7789_YELLOW :
      *color = ST7789_YELLOW;
      printf("ST7789_YELLOW\n");
      break;
    case ST7789_MAGENTA :
      *color = ST7789_MAGENTA;
      printf("ST7789_MAGENTA\n");
      break;
    case ST7789_CYAN :
      *color = ST7789_CYAN;
      printf("ST7789_CYAN\n");
      break;
    default :
      printf("another\n");
  }
}

void drawing_board(void) {
  //clean
  if (touch.chsc6x_read_touch_info(&touchX, &touchY) == 0) {
    if ((touchX > 210) && (touchX < 240) && (touchY < 40)) {
      Serial.printf("Clear screen touched! X: %d, Y: %d\r\n", touchX, touchY);
      delay(100);
      tft.fillScreen(ST7789_WHITE);
      draw_button_palette();
      isDrawing = false;
    }
    //Brush Size Selection
    else if ((touchX > 210) && (touchX < 240) && (touchY > 40) && (touchY < 70)) {
      Serial.println("Pen size button touched!");

      if (penSize == PEN_THIN) penSize = PEN_MEDIUM;
      else if (penSize == PEN_MEDIUM) penSize = PEN_THICK;
      else penSize = PEN_THIN;
      
      tft.fillRect(210, 40, 30, 30, ST7789_WHITE);
      tft.drawRect(210, 40, 30, 30, ST7789_BLACK);
      tft.fillCircle(225, 55, penSize, ST7789_BLACK);
      
      delay(200);
    }
    //Color Selection
    else if ((touchX > 210) && (touchX < 240) && (touchY > 70) && (touchY < 280)) {
      // choice_color();
      if(touchY>70 && touchY<100)        current_color = ST7789_BLACK;
      else if(touchY>100 && touchY<130)   current_color = ST7789_RED;
      else if(touchY>130 && touchY<160)   current_color = ST7789_GREEN;
      else if(touchY>160 && touchY<190)   current_color = ST7789_BLUE;
      else if(touchY>190 && touchY<220)   current_color = ST7789_YELLOW;
      else if(touchY>220 && touchY<250)   current_color = ST7789_MAGENTA;
      else if(touchY>250 && touchY<280)   current_color = ST7789_CYAN;

      choice_color(&current_color);
      delay(200);
    }
    else if ((touchX > 0 && touchX < 210) && (touchY > 0 && touchY < 320)) {
      if (!isDrawing) {
        lastX = touchX;
        lastY = touchY;
        isDrawing = true;
        draw_point(touchX, touchY, penSize, current_color);
      } else {
        draw_line(lastX, lastY, touchX, touchY, penSize, current_color);
        lastX = touchX;
        lastY = touchY;
      }
      Serial.printf("Drawing at X: %d, Y: %d\r\n", touchX, touchY);
    }
  } else {
    isDrawing = false;
  }
}

void draw_point(uint16_t x, uint16_t y, uint8_t size, uint16_t color) {
  if (size == 1) {
    tft.drawPixel(x, y, color);
  } else {
    tft.fillCircle(x, y, size, color);
  }
}

void draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t size, uint16_t color) {
  // Bresenham
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;
  
  while (true) {
    draw_point(x0, y0, size, color);
    
    if (x0 == x1 && y0 == y1) break;
    
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void VextON(void) {
  pinMode(Vext_Ctrl, OUTPUT);
  digitalWrite(Vext_Ctrl, LOW);
}

void blk_ctrl(bool state) {
  if(state == true) {
    pinMode(TFT_BLK, OUTPUT);
    digitalWrite(TFT_BLK, HIGH);
  }
  else {
    pinMode(TFT_BLK, LOW);
    digitalWrite(TFT_BLK, LOW);
  }

}
void VextOFF(void)  // Vext default OFF
{
  pinMode(Vext_Ctrl, OUTPUT);
  digitalWrite(Vext_Ctrl, HIGH);
}