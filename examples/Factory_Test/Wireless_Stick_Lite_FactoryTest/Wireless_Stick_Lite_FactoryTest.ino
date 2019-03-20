
  /*.........................................................................................
 * HelTec Automation(TM) WIFI_LoRa_32 factory test code, witch includ
 * follow functions:
 *
 * - Basic serial port test(in baud rate 115200);
 *
 * - LED blink test;
 *
 * - WIFI join and scan test;
 *
 * - LoRa Ping-Pong test(DIO0 -- GPIO26 interrup check the new incoming messages;
 *
 * - Timer test and some other Arduino basic functions.
 *
 *by Aaron.Lee from HelTec AutoMation, ChengDu, China
 *成都惠利特自动化科技有限公司
 *www.heltec.cn
 *
 *this project also realess in GitHub:
 *https://github.com/HelTecAutomation/Heltec_ESP32
*/
#include "Arduino.h"
#include "heltec.h"
#include "WiFi.h"

String packet;
uint64_t chipid;
bool receiveflag = false; // software flag for LoRa receiver, received data makes it true.

void WIFISetUp(void)
{
  //WIFI初始化 + 扫描演示
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.begin("WiFi SSID","WiFi password");
  delay(100);

  byte count = 0;
  Serial.print("Connecting.");
  while(WiFi.status() != WL_CONNECTED && count < 5)
  {
    count ++;
    Serial.print(".");
    delay(500);
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\r\nConnecting...OK.");
  }
  else
  {
    Serial.println("Connecting...Failed");
    //while(1);
  }
  Serial.println("WIFI Setup done");
}

void setup()
{
  Heltec.begin(false /*DisplayEnable Enable*/, true /*LoRa Enable*/, true /*Serial Enable*/, true /*LoRa use PABOOST*/, 868E6 /*LoRa RF working band*/);

  pinMode(25,OUTPUT);

  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);    //// OLED USE Vext as power supply, must turn ON Vext before OLED init
  delay(50);

  WIFISetUp();

  LoRa.beginPacket();
  LoRa.print(WiFi.scanNetworks());
  LoRa.print(" networks found");
  LoRa.endPacket();
  Serial.printf("LoRa data sent success!\r\n");

  // register the receive callback
  LoRa.onReceive(onReceive);

  // put the radio into receive mode
  LoRa.receive();
}

void loop()
{
  if(receiveflag)
  {
    chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
    Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
    Serial.printf("%08X\r\n",(uint32_t)chipid);//print Low 4bytes.

    receiveflag = false;
  }
  //delay(1000);
}

void onReceive(int packetSize)//LoRa receiver interrupt service
{
  packet = "";

  while (LoRa.available())
  {
      packet += (char) LoRa.read();
  }
  if((packet[0] == 'a')&&(packet[1] == 'b')&&(packet[2] == 'c'))
   {
    digitalWrite(25,HIGH);
    receiveflag = true;
   }
}
