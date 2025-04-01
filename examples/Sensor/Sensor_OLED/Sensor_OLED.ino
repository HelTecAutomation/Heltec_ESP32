// Example of Dual I2C Interface Configuration
// When using LoRa32 boards with built-in OLED displays (typically occupying I2C-0), connecting additional I2C sensors requires secondary interface initialization.
#include <Wire.h>          // I2C communication
#include <BMP280.h>        // BMP280 sensor
#include "HT_SSD1306Wire.h" // OLED display

// I2C instance for BMP280
TwoWire Wire2(1);

// Sensor and display objects
BMP280 bmp(&Wire2);
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

void setup() {
  Serial.begin(115200);     // Start serial for debugging
  delay(500);              // Stabilization delay
  
  Wire2.begin(42,41);      // Initialize I2C on pins 42(SDA),41(SCL)
  
  if(!bmp.begin()) {       // Initialize BMP280
    Serial.println("Sensor not found!");
    while(1);              // Halt if failed
  }
  
  delay(500);
  
  // Configure BMP280 settings
  bmp.setSampling(
    BMP280::MODE_NORMAL,     // Continuous measurement
    BMP280::SAMPLING_X2,     // Temp oversampling x2
    BMP280::SAMPLING_X16,    // Pressure oversampling x16  
    BMP280::FILTER_X16,      // Filter coefficient x16
    BMP280::STANDBY_MS_500   // 500ms standby
  );
  
  // OLED setup
  display.init();
  display.clear();
  display.display();
  display.setContrast(255);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "BMP280 Sensor Test");
  display.display();
}

void loop() {
  // Read sensor data
  float temp = bmp.readTemperature();          // Celsius
  float pressure = (float)bmp.readPressure() / 100.0; // hPa
  
  // Serial output
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C, Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  // OLED output
  display.clear();
  display.drawString(0, 20, "Temperature: " + String(temp) + " C");
  display.drawString(0, 40, "Pressure: " + String(pressure) + " hPa");
  display.display();
  
  delay(3000); // Update every 3 seconds
}