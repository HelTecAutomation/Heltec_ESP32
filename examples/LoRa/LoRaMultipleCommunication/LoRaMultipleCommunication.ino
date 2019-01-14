/*
  Heltec.LoRa Multiple Communication

  This example provide a simple way to achieve one to multiple devices
  communication.

  Each devices send datas in broadcast method. Make sure each devices
  working in the same BAND, then set the localAddress and destination
  as you want.
  
  Sends a message every half second, and polls continually
  for new incoming messages. Implements a one-byte addressing scheme,
  with 0xFD as the broadcast address. You can set this address as you
  want.

  Note: while sending, Heltec.LoRa radio is not listening for incoming messages.
  
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/
#include "heltec.h"

#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6


String outgoing;              // outgoing message

byte localAddress = 0xBB;     // address of this device
byte destination = 0xFD;      // destination to send to

byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

void setup()
{
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  Serial.println("Heltec.LoRa Duplex");

 
}

void loop()
{
  if (millis() - lastSendTime > interval)
  {
    String message = "Hello,I'm coming!";   // send a message
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(Heltec.LoRa.parsePacket());
}

void sendMessage(String outgoing)
{
  Heltec.LoRa.beginPacket();                   // start packet
  Heltec.LoRa.write(destination);              // add destination address
  Heltec.LoRa.write(localAddress);             // add sender address
  Heltec.LoRa.write(msgCount);                 // add message ID
  Heltec.LoRa.write(outgoing.length());        // add payload length
  Heltec.LoRa.print(outgoing);                 // add payload
  Heltec.LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize)
{
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = Heltec.LoRa.read();          // recipient address
  byte sender = Heltec.LoRa.read();            // sender address
  byte incomingMsgId = Heltec.LoRa.read();     // incoming msg ID
  byte incomingLength = Heltec.LoRa.read();    // incoming msg length

  String incoming = "";

  while (Heltec.LoRa.available())
  {
    incoming += (char)Heltec.LoRa.read();
  }

  if (incomingLength != incoming.length())
  {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(Heltec.LoRa.packetRssi()));
  Serial.println("Snr: " + String(Heltec.LoRa.packetSnr()));
  Serial.println();
}
