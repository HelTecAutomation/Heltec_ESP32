/*
 * HelTec Automation(TM) Wireless_Stick factory test code, witch include
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
 * 成都惠利特自动化科技有限公司
 * www.heltec.cn
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
*/
#include "heltec.h"
#include "WiFi.h"

#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6

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
  WiFi.begin("Your WiFi SSID","Your Password");//fill in "Your WiFi SSID","Your Password"
  delay(100);
  Heltec.display->clear();

  byte count = 0;
  while(WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    delay(500);
    Heltec.display->drawString(0, 0, "Connecting...");
    Heltec.display->display();
  }
  //Heltec.display->clear();
  if(WiFi.status() == WL_CONNECTED)
  {
    //Heltec.display->drawString(35, 38, "WIFI SETUP");
    Heltec.display->drawString(0, 9, "OK");
    Heltec.display->display();
    delay(1000);
    Heltec.display->clear();
  }
  else
  {
    //Heltec.display->clear();
    Heltec.display->drawString(0, 9, "Failed");
    Heltec.display->display();
    delay(1000);
    Heltec.display->clear();
  }
}

void WIFIScan(unsigned int value)
{
  unsigned int i;
  if(WiFi.status() != WL_CONNECTED)
  {
    WiFi.mode(WIFI_MODE_NULL);
  }  
  for(i=0;i<value;i++)
  {
    Heltec.display->drawString(0, 0, "Scan start...");
    Heltec.display->display();

    int n = WiFi.scanNetworks();
    Heltec.display->drawString(0, 9, "Scan done");
    Heltec.display->display();
    delay(500);
    Heltec.display->clear();

    if (n == 0)
    {
      Heltec.display->clear();
      Heltec.display->drawString(0, 18, "no network found");
      Heltec.display->display();
      //while(1);
    }
    else
    {
      Heltec.display->drawString(0, 18, (String)n + " nets found");
      Heltec.display->display();
      delay(2000);
      Heltec.display->clear();
    }
  }
}
bool resendflag=false;
bool deepsleepflag=false;
void interrupt_GPIO0()
{
  delay(10);
  if(digitalRead(0)==0)
  {
      if(digitalRead(LED)==LOW)
      {resendflag=true;}
      else
      {
        deepsleepflag=true;
      }     
  }
}
void setup()
{
  pinMode(LED,OUTPUT);
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /**/);

  WIFISetUp();
  WIFIScan(1);

  attachInterrupt(0,interrupt_GPIO0,FALLING);
  LoRa.onReceive(onReceive);
  send();
  LoRa.receive();
  displaySendReceive();
}

void loop()
{
 if(deepsleepflag)
 {
  LoRa.end();
  LoRa.sleep();
  delay(100);
  pinMode(4,INPUT);
  pinMode(5,INPUT);
  pinMode(14,INPUT);
  pinMode(15,INPUT);
  pinMode(16,INPUT);
  pinMode(17,INPUT);
  pinMode(18,INPUT);
  pinMode(19,INPUT);
  pinMode(26,INPUT);
  pinMode(27,INPUT);
  digitalWrite(Vext,HIGH);
  delay(2);
  esp_deep_sleep_start();
 }
 if(resendflag)
 {
   resendflag=false;
   send();      
   LoRa.receive();
   displaySendReceive();
 }
 if(receiveflag)
 {
    digitalWrite(25,HIGH);
    displaySendReceive();
    delay(1000);
    receiveflag = false;  
    send();
    LoRa.receive();
    displaySendReceive();
  }
}

void send()
{
    LoRa.beginPacket();
    LoRa.print("hello ");
    LoRa.print(counter++);
    LoRa.endPacket();
}
void displaySendReceive()
{
    Heltec.display->drawString(0, 0, (String)(counter-1) + " sent done");
    Heltec.display->drawString(0,9, "Received " + packSize);
    Heltec.display->drawString(0,16, packet);
    Heltec.display->drawString(0,24, rssi);
    Heltec.display -> display();
    delay(100);
    Heltec.display -> clear();
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
