/* Heltec Automation send communication test example
 *
 * Function:
 * 1. Send data from a esp32 device over hardware 
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

#if defined(WIFI_LORA_32_V3) || defined(WIFI_LORA_32_V2)
  SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
#endif

#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             22        // dBm

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

double txNumber;

bool lora_idle=true;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

void setup() {
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
	
    txNumber=0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
#if defined(WIRELESS_TRACKER_V2)
    st7735.st7735_init();
    st7735.st7735_fill_screen(ST7735_BLACK);
    st7735.st7735_write_str(0, 0, (String)"WIRELESS_TRACKER_V2");
    st7735.st7735_write_str(30, 0, (String)RF_FREQUENCY);
#endif
#if defined(WIFI_LORA_32_V3) || defined(WIFI_LORA_32_V2)
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
  delay(100);
	factory_display.init();
	factory_display.clear();
  factory_display.drawString(0, 0, (String)"WIFI_LORA_32_V3");
	factory_display.display();
#endif
}


uint32_t num = 0;
void loop()
{
  if(num>=100)
  {
    Serial.println("test over!");
#if defined(WIFI_LORA_32_V3) || defined(WIFI_LORA_32_V2)
    factory_display.clear();
    factory_display.drawString(0, 0, (String)"test over!");
    factory_display.display();
#endif
    while (1)
    {
      delay(1000);
    } 
  }

	if(lora_idle == true)
	{
    delay(100);
		txNumber += 0.01;
		sprintf(txpacket,"Hello %d number %0.2f",RF_FREQUENCY/1000000,txNumber);  //start a package
#if defined(WIRELESS_TRACKER_V2)
    st7735.st7735_write_str(0, 0, (String)txpacket);
#endif
#if defined(WIFI_LORA_32_V3) || defined(WIFI_LORA_32_V2)
    factory_display.clear();
    factory_display.drawString(0, 0, (String)txpacket);
    factory_display.display();
#endif
		Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
    lora_idle = false;
	}
  Radio.IrqProcess( );
}

void OnTxDone( void )
{
	Serial.println("TX done......");
	lora_idle = true;
  num++;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}