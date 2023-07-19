/*
  This is a simple example show the Heltec.LoRa recived data in OLED.

  The onboard OLED display is SSD1306 driver and I2C interface. In order to make the
  OLED correctly operation, you should output a high-low-high(1-0-1) signal by soft-
  ware to OLED's reset pin, the low-level signal at least 5ms.

  OLED pins to ESP32 GPIOs via this connecthin:
  OLED_SDA -- GPIO4
  OLED_SCL -- GPIO15
  OLED_RST -- GPIO16
  
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
  
  This project is also available on GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "string.h"
#include "stdio.h"
#include "HT_SSD1306Wire.h"
#include "images.h"
extern SSD1306Wire display;
#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

int16_t txNumber;

String  rssi,rxSize;

bool lora_idle = true;

void logo(){
 display.clear();
 display.drawXbm(0,5,logo_width,logo_height,logo_bits);
 display.display();
}

void LoRaData(){
 display.clear();
 display.setTextAlignment(TEXT_ALIGN_LEFT);
 display.setFont(ArialMT_Plain_10);
 display.drawString(0 , 15 , "Received "+ rxSize + " bytes");
 display.drawStringMaxWidth(0 , 26 , 128, rxpacket);
 display.drawString(0, 0, rssi);  
 display.display();
}

//void cbk(int packetSize) {
//  packet ="";
//  packSize = String(packetSize,DEC);
//  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
//  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
//  LoRaData();
//}

void setup() { 
   //WIFI Kit series V1 not support Vext control
  Mcu.begin();
 
 display.init();
 display.flipScreenVertically();  
 display.setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
 display.clear();
  
 display.drawString(0, 0, "Heltec.LoRa Initial success!");
 display.drawString(0, 10, "Wait for incoming data...");
 display.display();
  delay(1000);
  //LoRa.onReceive(cbk);
 Serial.begin(115200);
    Mcu.begin();
    
   
  
    RadioEvents.RxDone = OnRxDone;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                               LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                               LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                               0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
}

void loop() {
//  int packetSize = LoRa.parsePacket();
//  if (packetSize) { cbk(packetSize);  }
//  delay(10);
 if(lora_idle)
  {
    lora_idle = false;
    Serial.println("into RX mode");
    Radio.Rx(0);
  }
  Radio.IrqProcess( );
}
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );
    Serial.printf("\r\nreceived packet \"%s\" with rssi %d , length %d\r\n",rxpacket,rssi,rxSize);
    lora_idle = true;
}