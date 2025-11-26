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
#include "HT_SSD1306Wire.h"
#include "HT_st7735.h"

#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             0        // dBm

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

double txNumber;

bool lora_idle=true;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
#if defined(WIFI_LORA_32_V4)
SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
#endif
#if defined(WIRELESS_TRACKER_V2)
HT_st7735 st7735;
#endif
void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}
int8_t power = TX_OUTPUT_POWER;
bool interrupt_flag = false;
String powerStr,freqStr;
void interrupt_GPIO0(void)
{
	interrupt_flag = true;
}

void interrupt_handle(void)
{
	if(interrupt_flag)
	{
		interrupt_flag = false;
		if(digitalRead(0)==0)
		{
				delay(500);
				if(digitalRead(0)==0)
				{
	          digitalWrite(LED, HIGH);
            power += 1;
            Radio.SetTxConfig( MODEM_LORA, power, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
#if defined(WIFI_LORA_32_V4)
          factory_display.clear();
          powerStr = "power: "+String(power,DEC)+" dBm";
          freqStr = "freq: "+String(RF_FREQUENCY/1000000,DEC)+" MHz";
          factory_display.drawString(0, 0, powerStr);
          factory_display.drawString(0, 30, freqStr);
          factory_display.display();
#endif
#if defined(WIRELESS_TRACKER_V2)
			    st7735.st7735_fill_screen(ST7735_BLACK);
          powerStr = "power: "+String(power,DEC)+" dBm";
          freqStr = "freq: "+String(RF_FREQUENCY/1000000,DEC)+" MHz";
          st7735.st7735_write_str(0, 0, powerStr);
          st7735.st7735_write_str(0, 30, freqStr);
#endif
				}
		}
	}
}

void setup() {
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
		pinMode(LED ,OUTPUT);

    VextON();
#if defined(WIFI_LORA_32_V4)
    factory_display.init();
    factory_display.clear();
    factory_display.display();
    
    factory_display.clear();
    powerStr = "power: "+String(TX_OUTPUT_POWER,DEC)+" dBm";
    freqStr = "freq: "+String(RF_FREQUENCY/1000000,DEC)+" MHz";
    factory_display.drawString(0, 0, powerStr);
    factory_display.drawString(0, 30, freqStr);
    factory_display.display();
#endif
#if defined(WIRELESS_TRACKER_V2)
    st7735.st7735_init();
    st7735.st7735_fill_screen(ST7735_BLACK);
    powerStr = "power: "+String(power,DEC)+" dBm";
    freqStr = "freq: "+String(RF_FREQUENCY/1000000,DEC)+" MHz";
    st7735.st7735_write_str(0, 0, powerStr);
    st7735.st7735_write_str(0, 30, freqStr);
#endif
    txNumber=0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    attachInterrupt(0,interrupt_GPIO0,FALLING);

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
  }



void loop()
{
	digitalWrite(LED, LOW);
	if(lora_idle == true)
	{
    interrupt_handle();
    delay(1000);
		txNumber += 0.01;
		sprintf(txpacket,"Hello world number %0.2f",txNumber);  //start a package
   
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
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}  
