/* 
This factory test code will automatically identify screens of V1.1 and V1.2
*/
#include "Arduino.h"
#include "HT_lCMEN2R13EFC1.h"  // Display driver for ICMEN2R13EFC1
#include "HT_E0213A367.h"      // Display driver for E0213A367
#include "images.h"            // For image data (if used)

// Pin definitions for E-Paper display
#define PIN_EINK_SCLK 3    // Serial clock
#define PIN_EINK_DC   5    // Data/command control
#define PIN_EINK_CS   4    // Chip select
#define PIN_EINK_RES  6    // Reset pin
#define PIN_EINK_MOSI 2    // Master Out Slave In (data)
#define PIN_VEXT      45   // Vext enable pin (controls external power)

ScreenDisplay *factory_display;  // Pointer to display object
uint8_t displayChipId;           // Stores display controller chip ID

/**
 * @brief Enable external power by pulling Vext LOW
 */
void VextON() {
  pinMode(PIN_VEXT, OUTPUT);
  digitalWrite(PIN_VEXT, LOW);  // Active LOW to enable power
}

/**
 * @brief Disable external power by pulling Vext HIGH
 */
void VextOFF() {
  pinMode(PIN_VEXT, OUTPUT);
  digitalWrite(PIN_VEXT, HIGH);  // Deactivate external power
}

/**
 * @brief Display welcome screen with detected display info
 */
void displayWelcomeScreen() {
  factory_display->clear();
  
  // Display title
  factory_display->setFont(ArialMT_Plain_24);
  factory_display->setTextAlignment(TEXT_ALIGN_CENTER);
  factory_display->drawString(factory_display->width()/2, 5, "Screen Test");
  
  // Show detected display type
  factory_display->setFont(ArialMT_Plain_16);
  if ((displayChipId & 0x03) != 0x01) {
    factory_display->drawString(factory_display->width()/2, 35, "ICMEN2R13EFC1");
  } else {
    factory_display->drawString(factory_display->width()/2, 35, "E0213A367");
  }
  
  // Additional info
  factory_display->setFont(ArialMT_Plain_10);
  factory_display->drawString(factory_display->width()/2, 55, "Resolution: 250x122");
  factory_display->drawString(factory_display->width()/2, 70, "B/W e-ink");
  
  // Update and refresh display
  factory_display->update(BLACK_BUFFER);
  factory_display->display();
}

void setup() {
  Serial.begin(115200);
  
  // Enable external power first (critical for display operation)
  VextON();
  delay(100);  // Allow power to stabilize

  // Initialize display control pins
  pinMode(PIN_EINK_SCLK, OUTPUT); 
  pinMode(PIN_EINK_DC, OUTPUT); 
  pinMode(PIN_EINK_CS, OUTPUT);
  pinMode(PIN_EINK_RES, OUTPUT);
  
  // Reset sequence for E-Paper
  digitalWrite(PIN_EINK_RES, LOW);
  delay(20);
  digitalWrite(PIN_EINK_RES, HIGH);
  delay(20);

  // Prepare to send command (0x2F = read chip ID)
  digitalWrite(PIN_EINK_DC, LOW);  // Command mode
  digitalWrite(PIN_EINK_CS, LOW);  // Select display

  uint8_t cmd = 0x2F;
  pinMode(PIN_EINK_MOSI, OUTPUT);  
  digitalWrite(PIN_EINK_SCLK, LOW);
  
  // Send command bits MSB first
  for (int i = 0; i < 8; i++) {
    digitalWrite(PIN_EINK_MOSI, (cmd & 0x80) ? HIGH : LOW);
    cmd <<= 1;
    digitalWrite(PIN_EINK_SCLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_EINK_SCLK, LOW);
    delayMicroseconds(1);
  }
  delay(10);

  // Read chip ID response
  digitalWrite(PIN_EINK_DC, HIGH);  // Data mode
  pinMode(PIN_EINK_MOSI, INPUT_PULLUP); 
  displayChipId = 0;
  
  // Read 8-bit chip ID
  for (int8_t b = 7; b >= 0; b--) {
    digitalWrite(PIN_EINK_SCLK, LOW);  
    delayMicroseconds(1);
    digitalWrite(PIN_EINK_SCLK, HIGH);
    delayMicroseconds(1);
    if (digitalRead(PIN_EINK_MOSI)) displayChipId |= (1 << b);  
  }
  digitalWrite(PIN_EINK_CS, HIGH);  // Deselect display
  
  // Initialize appropriate display driver based on chip ID
  if ((displayChipId & 0x03) != 0x01) {
    factory_display = new HT_ICMEN2R13EFC1(6, 5, 4, 7, 3, 2, -1, 6000000);
    Serial.println("Detected HT_ICMEN2R13EFC1 display");
  } else {
    factory_display = new HT_E0213A367(6, 5, 4, 7, 3, 2, -1, 6000000);
    Serial.println("Detected HT_E0213A367 display");
  }

  // Initialize and display welcome screen
  factory_display->init();
  displayWelcomeScreen();
}

void loop() {
  // Main loop can be used for periodic updates
  // or to implement display refresh logic
}