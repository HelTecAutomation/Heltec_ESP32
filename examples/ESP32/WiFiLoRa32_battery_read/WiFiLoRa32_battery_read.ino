/*  
  ADC read voltage via GPIO1 simple test.
  Base on Wi_Fi LoRa 32 V3.2
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
*/

void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // Set the resolution of the analog-to-digital converter (ADC) to 12 bits (0-4095):
  analogReadResolution(12);

  // Set pin 37 as an output pin (used for ADC control):
  pinMode(37, OUTPUT);

  // Set pin 37 to HIGH (enable ADC control):
  digitalWrite(37, HIGH);
}

void loop() {
  // Read the raw analog value from pin 1 (range: 0-4095 for 12-bit resolution):
  int analogValue = analogRead(1);

  // Read the analog voltage in millivolts from pin 1:
  int analogVolts = analogReadMilliVolts(1);

  // Print the scaled analog value (scaled by a factor of 490/100):
  Serial.printf("ADC analog value = %d\n", analogValue * 490 / 100);

  // Print the scaled millivolts value (scaled by a factor of 490/100):
  Serial.printf("ADC millivolts value = %d\n", analogVolts * 490 / 100);

  // Add a delay of 1 second between readings for clear serial output:
  delay(1000);
}


