
  /*.........................................................................................
 * HelTec Automation(TM) Wireless Stick Lite factory test code, witch include
 * follow functions:
 *
 * - Node A search WiFi and send WiFi AP number to another node B via LoRa, Node B
 *   will send a string "abc" back to Node A, if Node A received "abc", test pass.
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
bool IsACKRecvied = false;


void WIFISetUp(void)
{
  //WIFI初始化 + 扫描演示
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.begin("WiFi SSID","WIFI password");
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

  WIFISetUp();

  // register the receive callback
  LoRa.onReceive(onReceive);

  // put the radio into receive mode
  LoRa.receive();
}

void loop()
{
	unsigned char WiFiCount = 0;

	if(!IsACKRecvied) //Scan native WiFi
	{
		WiFiCount = WiFi.scanNetworks();

		LoRa.beginPacket();
		LoRa.print("H");//data header
		LoRa.print(WiFiCount);
		LoRa.endPacket();
		Serial.printf("%d", WiFiCount);
		Serial.printf(" WiFi found\r\n");
	}

	// If received LoRa package with WIFI information, send ACK to node

	else if((packet[0] == 'H') && (packet[1] >= 0x31))
	{
		packet = "";

		digitalWrite(LED,LOW);

		Serial.printf("Received WiFi packet number.\r\n");

		LoRa.beginPacket();
		LoRa.print("abc");
		LoRa.endPacket();
		delay(10);

		Serial.printf("LoRa sent 'abc' done.\r\n");
	}

	else if((packet[0] == 'a') && (packet[1] == 'b') && (packet[2] == 'c')) // If received ACK package == "abc", test pass and print ChipID
	{
		packet = "";

		digitalWrite(LED,HIGH);

		chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
		Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
		Serial.printf("%08X\r\n",(uint32_t)chipid);//print Low 4bytes.

		digitalWrite(LED,HIGH);
	}

	LoRa.receive();
	delay(2000);
}

void onReceive(int packetSize)//LoRa receiver interrupt service
{
	packet = "";

	while (LoRa.available())
	{
		packet += (char) LoRa.read();
	}
	Serial.println(packet);
	Serial.println(LoRa.packetRssi());

	IsACKRecvied = true;
}
