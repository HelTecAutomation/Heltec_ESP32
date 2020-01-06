/*
 * Heltec Automation ESP32 Serial 1 & 2 example.
 * shows the usage of all 3 hardware uarts
 * work with ESP32's IO MUX
 * 
 * to test the software and hardware
 * wire Rx1 to Tx1 and Rx2 to Tx2
 * type text in serial monitor
 * text will walk trough all 2 serials
 * result is echoed to serial (usb)
*/


void setup() {
  Serial.begin(115200);

  // Serial1.begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
  // Serial2.begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
  // The txPin & rxPin can set to any output pin
  
  Serial1.begin(115200, SERIAL_8N1, 2, 17);
  Serial2.begin(115200, SERIAL_8N1, 22, 23);
}

void loop() {

  if(Serial.available()) {
    int ch = Serial.read();
    Serial1.write(ch);
  }

  if(Serial2.available()) {
    int ch = Serial2.read();
    Serial2.write(ch);
  }
  
  if(Serial2.available()) {
    int ch = Serial2.read();
    Serial.write(ch);
  }
}
