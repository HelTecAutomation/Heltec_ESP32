/*
 * HelTec Automation(TM) ESP32 Series Dev boards OLED Drawing Function test code
 *
 * - Some OLED Drawing Function function test;
 *
 * by lxyzn from HelTec AutoMation, ChengDu, China
 * 
 * www.heltec.cn
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
*/


// This example just provide basic function test;
// For more informations, please vist www.heltec.cn or mail to support@heltec.cn

#include "Arduino.h"
#include "heltec.h"


// Adapted from Adafruit_SSD1306
void drawLines() {
  for (int16_t i=0; i<DISPLAY_WIDTH; i+=4) {
    Heltec.display->drawLine(0, 0, i, DISPLAY_HEIGHT-1);
   Heltec.display->display();
    delay(10);
  }
  for (int16_t i=0; i<DISPLAY_HEIGHT; i+=4) {
    Heltec.display->drawLine(0, 0, DISPLAY_WIDTH-1, i);
    Heltec.display->display();
    delay(10);
  }
  delay(250);

  Heltec.display->clear();
  for (int16_t i=0; i<DISPLAY_WIDTH; i+=4) {
    Heltec.display->drawLine(0, DISPLAY_HEIGHT-1, i, 0);
    Heltec.display->display();
    delay(10);
  }
  for (int16_t i=DISPLAY_HEIGHT-1; i>=0; i-=4) {
    Heltec.display->drawLine(0, DISPLAY_HEIGHT-1, DISPLAY_WIDTH-1, i);
    Heltec.display->display();
    delay(10);
  }
  delay(250);

  Heltec.display->clear();
  for (int16_t i=DISPLAY_WIDTH-1; i>=0; i-=4) {
    Heltec.display->drawLine(DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1, i, 0);
    Heltec.display->display();
    delay(10);
  }
  for (int16_t i=DISPLAY_HEIGHT-1; i>=0; i-=4) {
    Heltec.display->drawLine(DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1, 0, i);
    Heltec.display->display();
    delay(10);
  }
  delay(250);
  Heltec.display->clear();
  for (int16_t i=0; i<DISPLAY_HEIGHT; i+=4) {
    Heltec.display->drawLine(DISPLAY_WIDTH-1, 0, 0, i);
    Heltec.display->display();
    delay(10);
  }
  for (int16_t i=0; i<DISPLAY_WIDTH; i+=4) {
    Heltec.display->drawLine(DISPLAY_WIDTH-1, 0, i, DISPLAY_HEIGHT-1);
    Heltec.display->display();
    delay(10);
  }
  delay(250);
}

// Adapted from Adafruit_SSD1306
void drawRect(void) {
  for (int16_t i=0; i<DISPLAY_HEIGHT/2; i+=2) {
    Heltec.display->drawRect(i, i, DISPLAY_WIDTH-2*i, DISPLAY_HEIGHT-2*i);
    Heltec.display->display();
    delay(10);
  }
}

// Adapted from Adafruit_SSD1306
void fillRect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<DISPLAY_HEIGHT/2; i+=3) {
    Heltec.display->setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
    Heltec.display->fillRect(i, i, DISPLAY_WIDTH - i*2, DISPLAY_HEIGHT - i*2);
    Heltec.display->display();
    delay(10);
    color++;
  }
  // Reset back to WHITE
  Heltec.display->setColor(WHITE);
}

// Adapted from Adafruit_SSD1306
void drawCircle(void) {
  for (int16_t i=0; i<DISPLAY_HEIGHT; i+=2) {
    Heltec.display->drawCircle(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, i);
    Heltec.display->display();
    delay(10);
  }
  delay(1000);
  Heltec.display->clear();

  // This will draw the part of the circel in quadrant 1
  // Quadrants are numberd like this:
  //   0010 | 0001
  //  ------|-----
  //   0100 | 1000
  //
  Heltec.display->drawCircleQuads(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, DISPLAY_HEIGHT/4, 0b00000001);
  Heltec.display->display();
  delay(200);
  Heltec.display->drawCircleQuads(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, DISPLAY_HEIGHT/4, 0b00000011);
  Heltec.display->display();
  delay(200);
  Heltec.display->drawCircleQuads(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, DISPLAY_HEIGHT/4, 0b00000111);
  Heltec.display->display();
  delay(200);
  Heltec.display->drawCircleQuads(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, DISPLAY_HEIGHT/4, 0b00001111);
  Heltec.display->display();
}

void printBuffer(void) {
  // Initialize the log buffer
  // allocate memory to store 8 lines of text and 30 chars per line.
  Heltec.display->setLogBuffer(5, 30);

  // Some test data
  const char* test[] = {
      "Hello",
      "World" ,
      "----",
      "Show off",
      "how",
      "the log buffer",
      "is",
      "working.",
      "Even",
      "scrolling is",
      "working"
  };

  for (uint8_t i = 0; i < 11; i++) {
    Heltec.display->clear();
    // Print to the screen
    Heltec.display->println(test[i]);
    // Draw it to the internal screen buffer
    Heltec.display->drawLogBuffer(0, 0);
    // Display it on the screen
    Heltec.display->display();
    delay(500);
  }
}

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  
  Heltec.display->setContrast(255);

  drawLines();
  delay(1000);
  Heltec.display->clear();

  drawRect();
  delay(1000);
  Heltec.display->clear();

  fillRect();
  delay(1000);
  Heltec.display->clear();

  drawCircle();
  delay(1000);
  Heltec.display->clear();

  printBuffer();
  delay(1000);
  Heltec.display->clear();
}

void loop() { }