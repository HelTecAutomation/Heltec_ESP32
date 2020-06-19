
#include "heltec.h"
#include "Arduino.h"

 SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED, RST_OLED);

void printBuffer(void) {
  // Initialize the log buffer
  // allocate memory to store 8 lines of text and 30 chars per line.
  display.setLogBuffer(5, 30);

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
    display.clear();
    // Print to the screen
    display.println(test[i]);
    // Draw it to the internal screen buffer
    display.drawLogBuffer(0, 0);
    // Display it on the screen
    display.display();
    delay(500);
  }
}

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

void setup() {

  VextON();
  delay(100);

  display.init();
  display.clear();
  display.display();
  
  display.setContrast(255);

  printBuffer();
  delay(1000);

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.clear();
  display.display();
  display.screenRotate(ANGLE_0_DEGREE);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 32-16/2, "ROTATE_0");
  display.display();
  delay(2000);

  display.clear();
  display.display();
  display.screenRotate(ANGLE_90_DEGREE);
  display.setFont(ArialMT_Plain_10);
  display.drawString(32, 64-10/2, "ROTATE_90");
  display.display();
  delay(2000);

  display.clear();
  display.display();
  display.screenRotate(ANGLE_180_DEGREE);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 32-16/2, "ROTATE_180");
  display.display();
  delay(2000);

  display.clear();
  display.display();
  display.screenRotate(ANGLE_270_DEGREE);
  display.setFont(ArialMT_Plain_10);
  display.drawString(32, 64-10/2, "ROTATE_270");
  display.display();
  delay(2000);
}

void loop() { }
