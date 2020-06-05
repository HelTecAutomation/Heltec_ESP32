/* Heltec Automation send communication test example
 *
 * Function:
 * 1. Send data from a CubeCell device over hardware 
 * 2. LoRa sending set frequency via serial
 * 
 * this example must work with Heltec ESP32 LoRaWAN library:
 * https://github.com/HelTecAutomation/ESP32_LoRaWAN
 * */

#include <ESP32_LoRaWAN.h>
#include "Arduino.h"

uint32_t  license[4] = {0x947025A9, 0x7E6C7FE4, 0x943C2CDA, 0xE04CD68B};

#define RF_FREQUENCY                                868000000 // Hz

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

double txNumber;

int16_t rssi,rxSize;
void  DoubleToString( char *str, double double_num,unsigned int len);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    SPI.begin(SCK,MISO,MOSI,SS);
    Mcu.init(SS,RST_LoRa,DIO0,DIO1,license);
    txNumber=0;
    rssi=0;

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
   }


uint32_t starttime = 0;
uint8_t data[128];
uint8_t dataIndex = 0;

uint32_t freq = 868000000;
void loop()
{
  starttime = millis();
  while( (millis() - starttime ) < 6000 )
  {
    if(Serial.available())
    {
      dataIndex = 0;
      while(Serial.available())
      {
        data[dataIndex++]=Serial.read();
        if(dataIndex == 128)
        {
          break;
        }
      }

      if(dataIndex == 5 && data[3] == '.')
      {
        if( data[0] <= '9' && data[0] >= '0' && data[1] <= '9' && data[1] >= '0' && data[2] <= '9' && data[2] >= '0' && data[4] <= '9' && data[4] >= '0' )
        {
          uint32_t freq = (data[0]-'0')*100*1000000 + (data[1]-'0')*10*1000000+(data[2]-'0')*1000000+(data[4]-'0')*100000;
          Serial.print("Freq set to ");
//          Serial.println(freq);
          Serial.write(data,dataIndex);
          Serial.println("MHz");
          Radio.Sleep();
           Radio.SetChannel( freq );
           Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
        }
        else
        {
          Serial.println("Input Error. Input example : 868.1");
        }
      }
      else
      {
        Serial.println("Input Error. Input example : 868.1");
      }
    }
  }
  
	txNumber += 0.01;
	sprintf(txpacket,"%s","Hello world number");  //start a package
//	sprintf(txpacket+strlen(txpacket),"%d",txNumber); //add to the end of package
	
//	DoubleToString(txpacket,txNumber,3);	   //add to the end of package

//	Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

//  Serial.println( Radio.TimeOnAir(MODEM_LORA, strlen(txpacket)) );
	Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
}

/**
  * @brief  Double To String
  * @param  str: Array or pointer for storing strings
  * @param  double_num: Number to be converted
  * @param  len: Fractional length to keep
  * @retval None
  */
void  DoubleToString( char *str, double double_num,unsigned int len) { 
  double fractpart, intpart;
  fractpart = modf(double_num, &intpart);
  fractpart = fractpart * (pow(10,len));
  sprintf(str + strlen(str),"%d", (int)(intpart)); //Integer part
  sprintf(str + strlen(str), ".%d", (int)(fractpart)); //Decimal part
}
