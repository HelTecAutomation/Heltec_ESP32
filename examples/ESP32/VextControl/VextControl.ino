/*
 * HelTec Automation(TM) Vext control example.
 *
 * Function summary:
 *
 * - Vext connected to 3.3V via a MOS-FET, the gate pin connected to GPIO21;
 *
 * - OLED display and PE4259(RF switch) use Vext as power supply;
 *
 * - WIFI Kit series V1 don't have Vext control function;
 *
 * HelTec AutoMation, Chengdu, China.
 * 成都惠利特自动化科技有限公司
 * www.heltec.cn
 * support@heltec.cn
 *
 * this project also release in GitHub:
 * https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/

#include "Arduino.h"
#include "heltec.h"
void setup()
{
	//WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
	//OLED use Vext power supply, Vext must be turn ON before OLED initialition.
	Heltec.display->init();

	Heltec.display->flipScreenVertically();
	Heltec.display->setFont(ArialMT_Plain_10);
	Heltec.display->drawString(0, 0, "Hello, I'm happy today");
	Heltec.display->display();
	delay(1000);
}

void loop()
{
	Heltec.display->sleep();//OLED sleep
	Heltec.VextON();
	Serial.println("Turn OFF Vext");
	delay(5000);

	Heltec.VextOFF();
	Serial.println("Turn ON Vext");
	Heltec.display->wakeup();
	delay(5000);
}
