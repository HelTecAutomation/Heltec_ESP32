#include "HT_lCMEN2R13EFC1.h"
#include "images.h"

// Initialize the display
HT_ICMEN2R13EFC1 display(6, 5, 4, 7, 3, 2, -1, 6000000); // rst,dc,cs,busy,sck,mosi,miso,frequency

#define DIRECTION ANGLE_0_DEGREE

int width, height;

void setup() {
  Serial.begin(115200);
  
  // Set display dimensions based on rotation
  if (DIRECTION == ANGLE_0_DEGREE || DIRECTION == ANGLE_180_DEGREE) {
    width = display._width;
    height = display._height;
  } else {
    width = display._height;
    height = display._width;
  }
  
  VextON();
  delay(100);

  // Initialize the display
  display.init();
  display.screenRotate(DIRECTION);
}

void drawImageDemo() {
    display.clear();
    display.update(BLACK_BUFFER);

    display.clear();
    int x = width/2 - WiFi_Logo_width/2;
    int y = height/2 - WiFi_Logo_height/2;
    display.drawXbm(x, y, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
    display.update(COLOR_BUFFER);
    display.display();
}

void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

void loop() {
  drawImageDemo();
  delay(15000); // Keep the logo displayed for 15 seconds
}
