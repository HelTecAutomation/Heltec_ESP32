
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

#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6

uint64_t chipid;
bool IsACKRecvied = false;

String rssi = "RSSI --";
String packSize = "--";
String packet;

unsigned int counter = 0;

void WIFISetUp(void)
{
  //WIFI初始化 + 扫描演示
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.begin("Your WiFi SSID","Your Password");//fill in "Your WiFi SSID","Your Password"
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

void WIFIScan(unsigned int value)
{
  unsigned int i;
  if(WiFi.status() != WL_CONNECTED)
  {
    WiFi.mode(WIFI_MODE_NULL);
  }

  for(i=0;i<value;i++)
  {
    Serial.println("Scan start...");

    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    delay(500);

    if (n == 0)
    {
      Serial.println("no network found");
      //while(1);
    }
    else
    {
      Serial.print(n);
      Serial.println("networks found:");
      delay(500);

      for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
        Serial.print((i + 1));
        Serial.print(":");
        Serial.print((String)(WiFi.SSID(i)));
        Serial.print(" (");
        Serial.print((String)(WiFi.RSSI(i)));
        Serial.println(")");;
        delay(10);
      }
    }
    delay(800);
  }
}

bool receiveflag = false;
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
}

void loop()
{
 if(deepsleepflag)
 {
  delay(1000);
  LoRa.end();
  LoRa.sleep();  
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
 }
 if(receiveflag)
 {
    digitalWrite(25,HIGH);
    Serial.print("Received Size ");
    Serial.print(packSize);
    Serial.print(" pakeges: ");
    Serial.print(packet);
    Serial.print(" With ");
    Serial.println(rssi);    
    receiveflag = false;  
    delay(1000);
    send();
    LoRa.receive();
  }
}

void send()
{
    LoRa.beginPacket();
    LoRa.print("hello ");
    LoRa.print(counter++);
    LoRa.endPacket();
    Serial.print("Packet ");
    Serial.print(counter-1);
    Serial.println(" sent done");
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
    
    rssi = "RSSI: " + String(LoRa.packetRssi(), DEC);

    receiveflag = true;
}