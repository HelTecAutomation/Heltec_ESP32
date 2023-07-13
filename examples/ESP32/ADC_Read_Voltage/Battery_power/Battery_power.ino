/*
 * HelTec Automation(TM) Electricity detection example.
 *
 * Function summary:
 *
 * - Vext connected to 3.3V via a MOS-FET, the gate pin connected to GPIO21;
 *
 * - Battery power detection is achieved by detecting the voltage of GPIO13;
 *
 * - OLED display and PE4259(RF switch) use Vext as power supply;
 *
 * - WIFI Kit series V1 don't have Vext control function;
 *
 * HelTec AutoMation, Chengdu, China.
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 * support@heltec.cn
 *
 * this project also release in GitHub:
 * https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
 * 
*/
#include "Arduino.h"
#include <Wire.h>
#include "heltec.h"

#define Fbattery    3700  //The default battery is 3700mv when the battery is fully charged.

float XS = 0.0025;      //The returned reading is multiplied by this XS to get the battery voltage.
uint16_t MUL = 1000;
uint16_t MMUL = 100;

void setup()
{
   Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);

	Heltec.display->init();
	Heltec.display->flipScreenVertically();
	Heltec.display->setFont(ArialMT_Plain_10);
	Heltec.display->drawString(0, 0, "OLED Start");
	Heltec.display->display();
	delay(1000);
	Heltec.display->clear();

   //analogSetClockDiv(255); // 1338mS
//   analogSetCycles(8);                   // Set number of cycles per sample, default is 8 and provides an optimal result, range is 1 - 255
//   analogSetSamples(1);                  // Set number of samples in the range, default is 1, it has an effect on sensitivity has been multiplied
   analogSetClockDiv(1);                 // Set the divider for the ADC clock, default is 1, range is 1 - 255
   analogSetAttenuation(ADC_11db);       // Sets the input attenuation for ALL ADC inputs, default is ADC_11db, range is ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
   analogSetPinAttenuation(36,ADC_11db); // Sets the input attenuation, default is ADC_11db, range is ADC_0db, ADC_2_5db, ADC_6db, ADC_11db
   analogSetPinAttenuation(37,ADC_11db);
                                        // ADC_0db provides no attenuation so IN/OUT = 1 / 1 an input of 3 volts remains at 3 volts before ADC measurement
                                        // ADC_2_5db provides an attenuation so that IN/OUT = 1 / 1.34 an input of 3 volts is reduced to 2.238 volts before ADC measurement
                                        // ADC_6db provides an attenuation so that IN/OUT = 1 / 2 an input of 3 volts is reduced to 1.500 volts before ADC measurement
                                        // ADC_11db provides an attenuation so that IN/OUT = 1 / 3.6 an input of 3 volts is reduced to 0.833 volts before ADC measurement
//   adcAttachPin(VP);                     // Attach a pin to ADC (also clears any other analog mode that could be on), returns TRUE/FALSE result 
//   adcStart(VP);                         // Starts an ADC conversion on attached pin's bus
//   adcBusy(VP);                          // Check if conversion on the pin's ADC bus is currently running, returns TRUE/FALSE result 
//   adcEnd(VP);
   
   adcAttachPin(36);
   adcAttachPin(37);

   Serial.begin(115200);
}

void loop()
{
   //WiFi LoRa 32        -- hardare versrion ≥ 2.3
   //WiFi Kit 32         -- hardare versrion ≥ 2
   //Wireless Stick      -- hardare versrion ≥ 2.3
   //Wireless Stick Lite -- hardare versrion ≥ 2.3
   //Battery voltage read pin changed from GPIO13 to GPI37
//   adcStart(37);
//   while(adcBusy(37));
   Serial.printf("Battery power in GPIO 37: ");
   Serial.println(analogRead(37));
   uint16_t c1  =  analogRead(37)*XS*MUL;
//   adcEnd(37);

   delay(100);

//   adcStart(36);
//   while(adcBusy(36));
   Serial.printf("voltage input on GPIO 36: ");
   Serial.println(analogRead(36));
   uint16_t c2  =  analogRead(36)*0.769 + 150;
//   adcEnd(36);
   Serial.println("-------------");
   // uint16_t c  =  analogRead(13)*XS*MUL;
   // Serial.println(analogRead(13));
   Heltec.display->drawString(0, 0, "Vbat = ");
   Heltec.display->drawString(33, 0, (String)c1);
   Heltec.display->drawString(60, 0, "(mV)");

   Heltec.display->drawString(0, 10, "Vin   = ");
   Heltec.display->drawString(33, 10, (String)c2);
   Heltec.display->drawString(60, 10, "(mV)");

   // Heltec.display->drawString(0, 0, "Remaining battery still has:");
   // Heltec.display->drawString(0, 10, "VBAT:");
   // Heltec.display->drawString(35, 10, (String)c);
   // Heltec.display->drawString(60, 10, "(mV)");
   Heltec.display->display();
   delay(5000);
   Heltec.display->clear();
}