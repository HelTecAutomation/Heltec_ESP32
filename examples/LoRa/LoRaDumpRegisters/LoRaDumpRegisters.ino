/*
  LoRa Register Dumper

  This examples shows how to inspect and output the LoRa radio's
  registers on the Serial interface

  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/

#include "heltec.h"

#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6

void setup() {
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  
  LoRa.dumpRegisters(Serial);
}


void loop() {
}
