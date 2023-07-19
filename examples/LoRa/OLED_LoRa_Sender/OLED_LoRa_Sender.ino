/*
  This is a simple example show the Heltec.LoRa sended data in OLED.

  The onboard OLED display is SSD1306 driver and I2C interface. In order to make the
  OLED correctly operation, you should output a high-low-high(1-0-1) signal by soft-
  ware to OLED's reset pin, the low-level signal at least 5ms.

  OLED pins to ESP32 GPIOs via this connecthin:
  OLED_SDA -- GPIO4
  OLED_SCL -- GPIO15
  OLED_RST -- GPIO16
  
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  https://heltec.org
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/

#include "LoRaWan_APP.h"
#include "Arduino.h"

#include "HT_SSD1306Wire.h"
#include "images.h"
#define RF_FREQUENCY                                868000000 // Hz

#define TX_OUTPUT_POWER                             15        // dBm

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
char Repacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnRxTimeout(void);
void displayMcuInit();
void displaySendReceive();

typedef enum
{
    STATUS_LOWPOWER,
    STATUS_RX,
    STATUS_TX
}States_t;


int16_t txNumber;
States_t state;
bool sleepMode = false;

uint32_t  license[4] = {0xD5397DF0, 0x8573F814, 0x7A38C73D, 0x48E68607};


extern SSD1306Wire display;
// Add your initialization code here
unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;
bool lora_idle=true;
void logo()
{
 display.clear();
 display.drawXbm(0,5,logo_width,logo_height,logo_bits);
 display.display();
}

void setup()
{
   //WIFI Kit series V1 not support Vext control
  Mcu.begin();
 
 display.init();
 display.flipScreenVertically();  
 display.setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
 display.clear();
  
 display.drawString(0, 0, "Heltec.LoRa Initial success!");
 display.display();
  delay(1000);
}

void loop()
{
 display.clear();
 display.setTextAlignment(TEXT_ALIGN_LEFT);
 display.setFont(ArialMT_Plain_10);
  
 display.drawString(0, 0, "Sending packet: ");
 display.drawString(90, 0, String(counter));
 display.display();

  // send packet
  // LoRa.beginPacket();
  
/*
 * LoRa.setTxPower(txPower,RFOUT_pin);
 * txPower -- 0 ~ 20
 * RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
 *   - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
 *   - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
*/
  // LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  // LoRa.print("hello ");
  // LoRa.print(counter);
  // LoRa.endPacket();
if(lora_idle == true)
  {
    delay(1000);
    txNumber += 0.01;
    sprintf(txpacket,"Hello  %d",counter);  //start a package
   
    Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

    Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
    lora_idle = false;
  }
  counter++;
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
