/* Heltec Automation  BH1750 Sensors test example
 *
 * Function:
 * Light intensity
 * 
 * HelTec AutoMation, Chengdu, China
 * www.heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
 * 
*/
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;


void setup(){

  Serial.begin(115200);

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));

}


void loop() {

  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  delay(1000);

}
