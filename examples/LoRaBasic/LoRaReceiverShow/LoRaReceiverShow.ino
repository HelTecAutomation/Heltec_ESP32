/* Heltec Automation Receive communication test example
 *
 * Function:
 * 1. Receive the same frequency band lora signal program
 *  
 * Description:
 * 
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
 * */

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_st7735.h"
#include "HT_SSD1306Wire.h"

#if defined(WIRELESS_TRACKER_V2)
  HT_st7735 st7735;
#endif

#if defined(WIFI_LORA_32_V3) || defined(WIFI_LORA_32_V4)
  SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
#endif

#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       11         // [SF7..SF12]
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

int16_t rssi,rxSize;

bool lora_idle = true;

void setup() {
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
    
    txNumber=0;
    rssi=0;
  
    RadioEvents.RxDone = OnRxDone;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                               LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                               LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                               0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
#if defined(WIRELESS_TRACKER_V2)
    st7735.st7735_init();
    st7735.st7735_fill_screen(ST7735_BLACK);
    st7735.st7735_write_str(0, 0, (String)"WIRELESS_TRACKER_V2");
    st7735.st7735_write_str(0, 60, (String)RF_FREQUENCY);
#endif

#if defined(WIFI_LORA_32_V3) || defined(WIFI_LORA_32_V4)
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
  delay(100);
	factory_display.init();
	factory_display.clear();
#if defined(WIFI_LORA_32_V4)
  factory_display.drawString(0, 0, (String)"WIFI_LORA_32_V4");
#else
  factory_display.drawString(0, 0, (String)"WIFI_LORA_32_V3");
#endif
  factory_display.drawString(0, 30, (String)RF_FREQUENCY);
	factory_display.display();
#endif
}

int32_t rec_num_all=0;
int32_t rec_rssi_all=0;
int32_t rec_snr_all=0;

void loop()
{
  if(lora_idle)
  {
    lora_idle = false;
    Serial.println("into RX mode");
    if(rec_num_all>0 && ((rec_num_all%10==0)||(rec_num_all>90)))
    {
      char show_buf[60];
      snprintf(show_buf,60,"num:%03d,rssi:%04d,snr:%03d",rec_num_all,rec_rssi_all/rec_num_all,rec_snr_all/rec_num_all);

#if defined(WIRELESS_TRACKER_V2)
      // st7735.st7735_fill_screen(ST7735_BLACK);
      st7735.st7735_write_str(0, 0, show_buf);
#endif
#if defined(WIFI_LORA_32_V3) || defined(WIFI_LORA_32_V4)
    factory_display.clear();
    factory_display.drawString(0, 0, show_buf);
    factory_display.display();
#endif
    }
    Radio.Rx(0);
  }
  Radio.IrqProcess();
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    rec_rssi_all += rssi;
    rec_snr_all += snr;
    rec_num_all ++;

    rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );
    Serial.printf("\r\nreceived packet \"%s\" with rssi %d , length %d\r\n",rxpacket,rssi,rxSize);
    lora_idle = true;
}