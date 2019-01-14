/*
 * HelTec Automation(TM) Wireless_Stick factory test code, witch includ
 * follow functions:
 * 
 * - Basic OLED function test;
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
 * by Aaron.Lee from HelTec AutoMation, ChengDu, China
 * 成都惠利特自动化科技有限格式
 * www.heltec.cn
 *
 * this project also realess in GitHub:
 * https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/
#include "heltec.h"
#include "WiFi.h"
#include "images.h"

String rssi = "RSSI --";
String packSize = "--";
String packet;

unsigned int counter = 0;

bool receiveflag = false; // software flag for LoRa receiver, received data makes it true.

long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends


#define RFOPIN false



void WIFISetUp(void)
{
	// Set WiFi to station mode and disconnect from an AP if it was previously connected
	WiFi.disconnect(true);
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.begin("HelTec_AutoMation","hunter_3120");//fill in "Your WiFi SSID","Your Password"
	delay(100);
  Heltec.display->clear();

	byte count = 0;
	while(WiFi.status() != WL_CONNECTED && count < 10)
	{
		count ++;
		delay(500);
    Heltec.display->drawString(32, 35, "Connecting...");
    Heltec.display->display();
	}
  //Heltec.display->clear();
  if(WiFi.status() == WL_CONNECTED)
  {
	  //Heltec.display->drawString(35, 38, "WIFI SETUP");
    Heltec.display->drawString(57, 48, "OK");
	  Heltec.display->display();
  	delay(1000);
    Heltec.display->clear();
  }
  else
  {
    //Heltec.display->clear();
    Heltec.display->drawString(50, 48, "Failed");
    Heltec.display->display();
    delay(1000);
    Heltec.display->clear();
  }
}

void WIFIScan(unsigned int value)
{
  unsigned int i;
  WiFi.mode(WIFI_MODE_NULL);
  for(i=0;i<value;i++)
  {
    Heltec.display->drawString(32, 32, "Scan start...");
    Heltec.display->display();

    int n = WiFi.scanNetworks();
    Heltec.display->drawString(32, 40, "Scan done");
    Heltec.display->display();
    delay(500);
    Heltec.display->clear();

     if (n == 0)
    {
     Heltec.display->clear();
     Heltec.display->drawString(32, 32, "no network found");
     Heltec.display->display();
     while(1);
    }
    else
    {
	   Heltec.display->drawString(32, 40, (String)n + " nets found");
	   Heltec.display->display();
		 delay(2000);
     Heltec.display->clear();
    }
  }
}

void setup()
{
	pinMode(LED,OUTPUT);
	 Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, 470E6 /**/);
  
	Heltec.display->init();
	Heltec.display->flipScreenVertically();
	Heltec.display->setFont(ArialMT_Plain_10);
	Heltec.display->clear();

  Heltec.display->drawString(48, 38, "TEST");
  Heltec.display->drawString(36, 48, "STARTING");
  Heltec.display->display();
  //delay(500);
  //Heltec.display->clear();

	WIFISetUp();
	WIFIScan(1);
	
  //LoRa.setTxPower(10,RFOPIN);
	Heltec.display->drawString(35, 40, "LoRa TEST");
	Heltec.display->display();
	delay(300);
	Heltec.display->clear();

	// register the receive callback
	LoRa.onReceive(onReceive);

	// put the radio into receive mode
	LoRa.receive();
}

void loop()
{
	if(millis() - lastSendTime > interval)//waiting LoRa interrupt
	{
		LoRa.beginPacket();
		LoRa.print("hello ");
		LoRa.print(counter++);
		LoRa.endPacket();

		LoRa.receive();

		digitalWrite(LED,HIGH);
		Heltec.display->drawString(32, 54, (String)(counter-1) + " sent done");
		Heltec.display->display();

		interval = random(1000) + 1000; //1~2 seconds
		lastSendTime = millis();

    Heltec.display->clear();
	}
	if(receiveflag)
	{
		Heltec.display->drawString(32,29, "Received " + packSize);
		Heltec.display->drawString(32,38, packet);
		Heltec.display->drawString(32,47, rssi);
		Heltec.display->display();

		digitalWrite(LED,LOW);

		receiveflag = false;
	}
	//delay(1000);
}

void onReceive(int packetSize)//LoRa receiver interrupt service
{
	//if (packetSize == 0) return;

	packet = "";
    packSize = String(packetSize,DEC);

    while (LoRa.available())
    {
    		packet += (char) LoRa.read();
    }

    Serial.println(packet);
    rssi = "RSSI: " + String(LoRa.packetRssi(), DEC);

    receiveflag = true;
}