/* Heltec Automation  BMP180 Sensors test example
 *
 * Function:
 * Air pressure sensor
 * 
 * HelTec AutoMation, Chengdu, China
 * www.heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
 * 
*/
#include "Arduino.h"
#include "heltec.h" 
#include <Wire.h>
#include <BMP180.h>
#include "string.h"

BMP085 bmp;

uint8_t T[20] = {"Temperature"};

void setup() {
  Serial.begin(115200);
}
void drawFontFaceDemo(double T,double P,double A,double R) {
    Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
//  Heltec.display->flipScreenVertically();
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0   , 0  ,   "Temperature =");
    Heltec.display->drawString(76  , 0  ,   (String)T);
    Heltec.display->drawString(106 , 0  ,   " *C");
    Heltec.display->drawString(0   , 16 ,   "Pressure =");
    Heltec.display->drawString(56  , 16 ,   (String)P);
    Heltec.display->drawString(100 , 16 ,    "Pa");
    Heltec.display->drawString(0   , 32 ,   "Altitude   =");
    Heltec.display->drawString(56  , 32 ,   (String)A);
    Heltec.display->drawString(86 , 32 ,   " m");
    Heltec.display->drawString(0   , 48 ,   "Real altitude =");
    Heltec.display->drawString(76  , 48 ,   (String)R);
    Heltec.display->drawString(106 , 48 ,   " m");
    Heltec.display->display();
}  
void loop() {
    if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  while (1) {}
  }
    double T =bmp.readTemperature();
    Serial.print("Temperature = ");
    Serial.print(T);
    Serial.println(" *C");

    double P =  bmp.readPressure();
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    double A = bmp.readAltitude();
    // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");

    
    Serial.print("Pressure at sealevel (calculated) = ");
    Serial.print(bmp.readSealevelPressure());
    Serial.println(" Pa");

    double R = bmp.readAltitude(101500);
  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" meters");
    
    Serial.println();
    delay(500);
	drawFontFaceDemo(T, P, A, R) ;
	delay(5000);
}
