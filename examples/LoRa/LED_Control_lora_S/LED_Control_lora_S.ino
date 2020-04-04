/* 
  Send a lora signal to control the LED light switch of another board.
  Pull the 12th pin high to turn on the LED light.
  Pull pin 13 high to turn off the LED light.
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/
#include "heltec.h"
#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6

int counter = 0;
#define Open_LED  12
#define Close_LED 13
void setup() {
  
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(false /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
	pinMode(Open_LED,INPUT);
    digitalWrite(Open_LED,LOW);
	pinMode(Close_LED,INPUT);
    digitalWrite(Close_LED,LOW);
	LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  
}

void loop() {
	if(digitalRead(Open_LED)){
	  Serial.print("Sending packet: OpenLED\r\n");
	  // send packet
	  LoRa.beginPacket();
	  LoRa.print("OpenLED");
	  LoRa.endPacket();
	  digitalWrite(Open_LED,LOW);                       
	}
	if(digitalRead(Close_LED)){
		Serial.print("Sending packet: CloseLED\r\n");
		// send packet
		LoRa.beginPacket();
		LoRa.print("CloseLED");
		LoRa.endPacket();
		digitalWrite(Close_LED,LOW);  
	}
	delay(1000);  
}
