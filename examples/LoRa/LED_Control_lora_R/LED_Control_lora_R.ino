/* 
  Receive the lora signal to control the on and off of the LED.
  Pull the 12th pin high to turn on the LED light.
  Pull pin 13 high to turn off the LED light.
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/

#include "heltec.h"
#include "string.h"
#include "stdio.h"
#define LED 25
#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6
char Readback[50];
void setup() {
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(false /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
	pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW); 
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    // read packet
    while (LoRa.available()) {
	  sprintf(Readback+strlen(Readback),"%c",(char)LoRa.read());
    }
    Serial.print(Readback);
	if(strncmp(Readback, "OpenLED", strlen(Readback)) == 0) {
		digitalWrite(LED, HIGH); 
	 }
	else if(strncmp(Readback, "CloseLED", strlen(Readback)) == 0) {
		digitalWrite(LED, LOW); 
	 }
	 memset(Readback,0,50);
    // print RSSI of packet
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
