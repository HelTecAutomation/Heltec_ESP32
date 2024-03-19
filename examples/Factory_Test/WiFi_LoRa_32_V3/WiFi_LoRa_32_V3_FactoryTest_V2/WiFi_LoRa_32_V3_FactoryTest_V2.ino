/*
 * HelTec Automation(TM) WIFI_LoRa_32 factory test code, witch includ
 * follow functions:
 * 
 * - Basic OLED function test;
 * 
 * - Basic serial port test(in baud rate 115200);
 * 
 * - LED blink test;
 * 
 * - WIFI connect and scan test;
 * 
 * - LoRa Ping-Pong test (DIO0 -- GPIO26 interrup check the new incoming messages);
 * 
 * - Timer test and some other Arduino basic functions.
 *
 * by Aaron.Lee from HelTec AutoMation, ChengDu, China
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
*/

#include "Arduino.h"
#include "WiFi.h"
#include "images.h"
#include "LoRaWan_APP.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"
/********************************* lora  *********************************************/
#define RF_FREQUENCY_1                                868500000 // Hz
#define RF_FREQUENCY_2                                870500000 // Hz

#define TX_OUTPUT_POWER                             10        // dBm

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

#define USERKEY 0
#define TEST_WIFI_SSID       "TP-LINK_B8BC"
#define TEST_WIFI_PWD        "heltec_test"

SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

uint8_t txpacket[BUFFER_SIZE];
uint8_t rxpacket[BUFFER_SIZE];


static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

typedef enum
{
    LOWPOWER,
    STATE_RX,
    STATE_TX
}States_t;

States_t state;
int16_t maxTxRssi=-255;
int16_t maxRxRssi=-255;

String packSize = "--";
String packet;

bool receiveflag = false; // software flag for LoRa receiver, received data makes it true.
uint64_t chipid;
uint8_t rx_cnt=0;
int test_mode=0;
int wifiScanCnt=0;
int wifiScanMaxRssi=-255;
bool wifiConnected=false;
bool loratimeout=false;
extern uint32_t ex_32k_start_cnt;
void OnTxDone( void )
{
	Serial.println("TX done......");
	state=STATE_RX;
}

void OnTxTimeout( void )
{
  Radio.Sleep( );
  Serial.println("TX Timeout......");
	state=STATE_TX;
}
void showStatus(int value)
{
  Serial.printf("WiFi %d %d %d\r\n",wifiConnected,wifiScanCnt,wifiScanMaxRssi);
  Serial.printf("LoRa %d %d\r\n",maxTxRssi,maxRxRssi);
  factory_display.clear();
  factory_display.setFont(ArialMT_Plain_10);
  factory_display.setTextAlignment(TEXT_ALIGN_LEFT);
  packet ="WIFI   ";
  if(wifiConnected)
  {
    packet+="OK";
  }
  else
  {
    packet+="XX";
  }
  factory_display.drawString(0, 0, packet);

  packet ="WIFI Scan Cnt:"+String(wifiScanCnt)+" Rssi:"+String(wifiScanMaxRssi);
  factory_display.drawString(0, 15, packet);

  if(value)
  {
    packet ="LoRa Tx Rssi : "+String(maxTxRssi);
    factory_display.drawString(0, 30, packet);
  
    packet ="LoRa Rx Rssi : "+String(maxRxRssi);
    factory_display.drawString(0, 45, packet);
  }
  else
  {
    packet ="LoRa Error";
    factory_display.drawString(0, 45, packet);
  }
  uint32_t t=millis()/1000;
  
  factory_display.setTextAlignment(TEXT_ALIGN_RIGHT);
  packet ="time:"+String(t);
  factory_display.drawString(128, 0, packet);
  
  factory_display.display();
  digitalWrite(LED, 1);  
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  Radio.Sleep( );
  Serial.println("RX done......");
  Serial.printf("Rx size : %d , rssi : %d, snr : %d\r\n",size,rssi,snr);
//  for(int i=0;i<size;i++)
//  {
//    Serial.printf("%02X ",payload[i]);
//  }
//  Serial.println();
  if(test_mode==0)
  {
    state=STATE_TX;
    if(size==10)
    {
      uint64_t rxchipid=*((uint64_t *)payload);
      if(rxchipid==chipid)
      {
        int16_t txrssi=*(int16_t *)(payload+8);
        Serial.printf("rx own mes,rssi :%d\r\n",txrssi);
        if(maxTxRssi<txrssi)
          maxTxRssi=txrssi;
        if(maxRxRssi<rssi)
          maxRxRssi=rssi;
        rx_cnt++;
        if(rx_cnt>=3)
        {
          showStatus(1);
          state=LOWPOWER;
        }
      }
    }
  }
  else
  {
    if(size==8)
    {
      state=STATE_TX;
      memcpy(txpacket,payload,8);
      memcpy(txpacket+8,(uint8_t *)&rssi,2);
    }
    else
    {
      Radio.Rx(0);
    }
  }
}

void OnRxTimeout()
{
  if(test_mode==0)
  {
    Radio.Sleep();
    state=STATE_TX;
    Serial.println("RX Timeout......");
  }
}

void OnRxError()
{
  if(test_mode==0)
  {
    Radio.Sleep();
    state=STATE_TX;
    Serial.println("RX Error......");
  }
}

void lora_init(void)
{
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.RxError = OnRxError;
  RadioEvents.RxTimeout = OnRxTimeout;
  Radio.Init( &RadioEvents );
  srand1( Radio.Random( ) );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                 LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                 LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
	state=STATE_TX;
}



void logo(){
	factory_display.clear();
	factory_display.drawXbm(0,5,logo_width,logo_height,(const unsigned char *)logo_bits);
	factory_display.display();
}

void WIFISetUp(void)
{
	// Set WiFi to station mode and disconnect from an AP if it was previously connected
	WiFi.disconnect(true);
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.begin(TEST_WIFI_SSID,TEST_WIFI_PWD);//fill in "Your WiFi SSID","Your Password"
	delay(100);

	byte count = 0;
	while(WiFi.status() != WL_CONNECTED && count < 10)
	{
		count ++;
		delay(500);
		factory_display.drawString(0, 0, "Connecting...");
		factory_display.display();
	}

	factory_display.clear();
	if(WiFi.status() == WL_CONNECTED)
	{
    wifiConnected=true;
		factory_display.drawString(0, 0, "Connecting...OK.");
		factory_display.display();
//		delay(500);
	}
	else
	{
		factory_display.clear();
		factory_display.drawString(0, 0, "Connecting...Failed");
		factory_display.display();
		//while(1);
	}
	factory_display.drawString(0, 10, "WIFI Setup done");
	factory_display.display();
	delay(500);
}

void WIFIScan(unsigned int value)
{
	unsigned int i;
  WiFi.mode(WIFI_STA);

	for(i=0;i<value;i++)
	{
		factory_display.drawString(0, 20, "Scan start...");
		factory_display.display();

		int n = WiFi.scanNetworks();
    wifiScanCnt=n;
		factory_display.drawString(0, 30, "Scan done");
		factory_display.display();
		delay(500);
		factory_display.clear();

		if (n == 0)
		{
			factory_display.clear();
			factory_display.drawString(0, 0, "no network found");
			factory_display.display();
			//while(1);
		}
		else
		{
			factory_display.drawString(0, 0, (String)n);
			factory_display.drawString(14, 0, "networks found:");
			factory_display.display();
			delay(500);

			for (int i = 0; i < n; ++i) {
			// Print SSID and RSSI for each network found
        if(wifiScanMaxRssi<WiFi.RSSI(i))
        {
          wifiScanMaxRssi=WiFi.RSSI(i);
        }
				factory_display.drawString(0, (i+1)*9,(String)(i + 1));
				factory_display.drawString(6, (i+1)*9, ":");
				factory_display.drawString(12,(i+1)*9, (String)(WiFi.SSID(i)));
				factory_display.drawString(90,(i+1)*9, " (");
				factory_display.drawString(98,(i+1)*9, (String)(WiFi.RSSI(i)));
				factory_display.drawString(114,(i+1)*9, ")");
				//factory_display.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
				delay(10);
			}
		}

		factory_display.display();
		delay(800);
		factory_display.clear();
	}
}

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

void intodeepsleep()
{
  Serial.println("into deep sleep");
  VextOFF();
  Radio.Sleep();
  SPI.end();
  pinMode(RADIO_DIO_1,ANALOG);
  pinMode(RADIO_NSS,ANALOG);
  pinMode(RADIO_RESET,ANALOG);
  pinMode(RADIO_BUSY,ANALOG);
  pinMode(LORA_CLK,ANALOG);
  pinMode(LORA_MISO,ANALOG);
  pinMode(LORA_MOSI,ANALOG);
  esp_sleep_enable_timer_wakeup(600*1000*(uint64_t)1000);
  esp_deep_sleep_start();
}
TaskHandle_t checkUserkey1kHandle = NULL;

void checkUserkey(void *pvParameters)
{
  uint32_t keydowntime;
  pinMode(USERKEY,INPUT);
  while(1)
  {
    if(digitalRead(USERKEY)==0)
    {
      keydowntime=millis();
      Serial.printf("key down : %u\r\n",keydowntime);
      delay(10);
      while(digitalRead(USERKEY)==0){
        if( (millis()-keydowntime)>3000 )
        {
          break;
        }
      }
      if( (millis()-keydowntime)>3000 )
      {
        Serial.println("into lora mode 1");
        test_mode=1;
        factory_display.clear();
        factory_display.setFont(ArialMT_Plain_16);
        factory_display.setTextAlignment(TEXT_ALIGN_CENTER);
        packet ="LORA MODE "+String(test_mode);
        factory_display.drawString(64, 24, packet);
        factory_display.display();
        delay(1000);
        factory_display.setFont(ArialMT_Plain_10);
        factory_display.setTextAlignment(TEXT_ALIGN_LEFT);
        state=STATE_RX;
      }
      else
      {
        intodeepsleep();
      }
    }
  }
}
void setup()
{
  Serial.begin(115200);
  if(ex_32k_start_cnt>5)
  {
    VextON();
    delay(100);
    factory_display.init();
    factory_display.setFont(ArialMT_Plain_16);
    factory_display.setTextAlignment(TEXT_ALIGN_CENTER);
    packet ="EX 32K ERROR";
    factory_display.clear();
    factory_display.drawString(64, 24, packet);
    factory_display.display();
    while(1);
  }
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\r\n",(uint32_t)chipid);//print Low 4bytes.
  xTaskCreateUniversal(checkUserkey, "checkUserkey1Task", 2048, NULL, 1, &checkUserkey1kHandle, CONFIG_ARDUINO_RUNNING_CORE);
  VextON();
  delay(100);
  factory_display.init();
  factory_display.fillRect(0, 0, factory_display.getWidth(), factory_display.getHeight());
  factory_display.display();
  delay(1000);
	logo();
  delay(1000);
	factory_display.clear();
  lora_init();
	WIFISetUp();
	WiFi.disconnect(); //
	WiFi.mode(WIFI_STA);
	delay(100);

	WIFIScan(1);

  factory_display.setFont(ArialMT_Plain_16);
  factory_display.setTextAlignment(TEXT_ALIGN_CENTER);
	packet ="LORA MODE "+String(test_mode);
  factory_display.drawString(64, 24, packet);
  factory_display.display();
  delay(100);
  factory_display.clear();
	pinMode(LED ,OUTPUT);
	digitalWrite(LED, LOW);  
}


void loop()
{
  if(test_mode)
  {
    switch(state)
    {
      case STATE_TX:
        Radio.SetChannel( RF_FREQUENCY_2 );
        Serial.println("loraMode 1:into TX mode");
        Radio.Send( txpacket, 10 );
        state=LOWPOWER;
        break;
      case STATE_RX:
        Radio.SetChannel( RF_FREQUENCY_1 );
        Serial.println("loraMode 1:into RX mode");
        Radio.Rx( 0 );
        state=LOWPOWER;
        break;
      case LOWPOWER:
        break;
      default:
        break;
    }
  }
  else
  {
    if(millis()>120000 && rx_cnt<3)
    {
      if(loratimeout==false)
      {
        loratimeout=true;
        showStatus(0);
      }
      return;
    }
    switch(state)
    {
      case STATE_TX:
        delay(2000+randr(0,1000));
        Radio.SetChannel( RF_FREQUENCY_1 );
        Serial.println("loraMode 0:into TX mode");
        Radio.Send( (uint8_t *)&chipid, 8 );
        state=LOWPOWER;
        break;
      case STATE_RX:
        Radio.SetChannel( RF_FREQUENCY_2 );
        Serial.println("loraMode 0:into RX mode");
        Radio.Rx( 1000 );
        state=LOWPOWER;
        break;
      case LOWPOWER:
        break;
      default:
        break;
    }
  }
  Mcu.timerhandler();
  Radio.IrqProcess( );
}
