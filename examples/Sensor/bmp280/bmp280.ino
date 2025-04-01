#include "Arduino.h"
#include <Wire.h>
#include <BMP280.h>

BMP280 bmp;

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin(42,41);
  bmp.begin();
  delay(500);
  bmp.setSampling(BMP280::MODE_NORMAL,     /* Operating Mode. */
                  BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  BMP280::FILTER_X16,      /* Filtering. */
                  BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop()
{
  float temp = bmp.readTemperature();
  float Pressure = (float)bmp.readPressure() / 100.0;
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C, Pressure: ");
  Serial.print(Pressure);
  Serial.println(" hPa");
  delay(1000);
}