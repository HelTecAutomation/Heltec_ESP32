/*
 * HelTec Automation(TM) LoRaWAN 1.0.2 OTAA example use OTAA, CLASS A
 *
 * Function summary:
 *
 * - use internal RTC(150KHz);
 *
 * - Include stop mode and deep sleep mode;
 *
 * - 15S data send cycle;
 *
 * - Informations output via serial(115200);
 *
 * - Only ESP32 + LoRa series boards can use this library, need a license
 *   to make the code run(check you license here: http://www.heltec.cn/search/);
 *
 * You can change some definition in "Commissioning.h" and "LoRaMac-definitions.h"
 *
 * HelTec AutoMation, Chengdu, China.
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 * support@heltec.cn
 *
 *this project also release in GitHub:
 *https://github.com/HelTecAutomation/ESP32_LoRaWAN
*/

#include <ESP32_LoRaWAN.h>
#include "Arduino.h"

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
int16_t Rssi,rxSize;

uint32_t  license[4] = {0xD5397DF0, 0x8573F814, 0x7A38C73D, 0x48E68607};

// Add your initialization code here
void setup()
{
  displayMcuInit();
  
  Serial.begin(115200);
  while (!Serial);
  SPI.begin(SCK,MISO,MOSI,SS);
  Mcu.init(SS,RST_LoRa,DIO0,DIO1,license);


    txNumber=0;
    Rssi=0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
    state=STATUS_TX;
}


void loop()
{
  switch(state)
  {
    case STATUS_TX:
      delay(1000);
      txNumber++;
        sprintf(txpacket,"%s","hello");
        sprintf(txpacket+strlen(txpacket),"%d",txNumber);
        sprintf(txpacket+strlen(txpacket),"%s"," Rssi : ");
        sprintf(txpacket+strlen(txpacket),"%d",Rssi);

        Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

        Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
        state=STATUS_LOWPOWER;
        break;
    case STATUS_RX:
        Serial.println("into RX mode");
        Radio.Rx( 5000 );
        state=STATUS_LOWPOWER;
        break;
    case STATUS_LOWPOWER:
        LoRaWAN.sleep(CLASS_C,0);
        break;
    default:
        break;
  }
}

void OnTxDone( void )
{
  Serial.print("TX done......");
  sprintf(Repacket,"%s","");
  displaySendReceive();
  state=STATUS_RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.print("TX Timeout......");
    sprintf(Repacket,"%s","TX Timeout,Retransmission");
    displaySendReceive();
    state=STATUS_TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );

    Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n",rxpacket,Rssi,rxSize);
    Serial.println("wait to send next packet");
    sprintf(Repacket,"%s","");
    displaySendReceive();

    state=STATUS_TX;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    Serial.print("RX Timeout......");
    sprintf(Repacket,"%s","RX Timeout,Retransmission");
    displaySendReceive();
    state=STATUS_TX;
}

void displayMcuInit()
{
  Display.wakeup();
  Display.init();
  delay(100);
  Display.flipScreenVertically();
  Display.setFont(ArialMT_Plain_10);
  Display.setTextAlignment(TEXT_ALIGN_LEFT);
  Display.clear();
  Display.drawString(18, 11, "ESP32_PINGPONG");
  Display.drawString(40, 31, "STARTING");

  Display.display();
  delay(100);
  Display.clear();
}

void displaySendReceive()
{
    Display.drawString(0,0,"sending packet:");
    Display.drawString(0,11,(String)txpacket);
    Display.drawString(0,25,(String)Repacket);
    Display.drawString(0,40,"received packet:" );
    Display.drawString(0,51,(String)rxpacket);
    Display.display();
    delay(100);
    Display.clear();
}
