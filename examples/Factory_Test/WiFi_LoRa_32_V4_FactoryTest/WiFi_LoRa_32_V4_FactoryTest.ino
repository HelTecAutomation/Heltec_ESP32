/*
 * HelTec Automation(TM) WiFi_LoRa_32_V4_FactoryTest factory test code, witch includ
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
#include "LoRaWan_APP.h"
#include <Wire.h>  
#include "HT_TinyGPS++.h"
#include "HT_SSD1306Wire.h"
#include "images.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"

typedef enum 
{
	WIFI_CONNECT_TEST_INIT,
	WIFI_CONNECT_TEST,
	WIFI_SCAN_TEST,
	LORA_TEST_INIT,
	LORA_COMMUNICATION_TEST,
	DEEPSLEEP_TEST,
	GPS_TEST,
}test_status_t;

SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
TinyGPSPlus gps;
#define VGNSS_CTRL 34
test_status_t  test_status;
uint16_t wifi_connect_try_num = 15;
bool resendflag=false;
bool deepsleepflag=false;
bool interrupt_flag = false;
/********************************* lora  *********************************************/
#define RF_FREQUENCY                                868000000 // Hz

#define TX_OUTPUT_POWER                             20        // dBm

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
void OnTxDone( void );
void OnTxTimeout( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

typedef enum
{
    LOWPOWER,
    STATE_RX,
    STATE_TX
}States_t;

int16_t txNumber = 0;
int16_t rxNumber = 0;
States_t state;
bool sleepMode = false;
int16_t Rssi,rxSize;

String rssi = "RSSI --";
String packet;
String send_num;

unsigned int counter = 0;
bool receiveflag = false; // software flag for LoRa receiver, received data makes it true.
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends
uint64_t chipid;
int16_t RssiDetection = 0;


void OnTxDone( void )
{
	Serial.print("TX done......");
	state=STATE_RX;
}

void OnTxTimeout( void )
{
	Radio.Sleep( );
	Serial.print("TX Timeout......");
	state=STATE_TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	rxNumber++;
	Rssi=rssi;
	rxSize=size;
	memcpy(rxpacket, payload, size );
	rxpacket[size]='\0';
	Radio.Sleep( );
	Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n",rxpacket,Rssi,rxSize);
	Serial.println("wait to send next packet");
	receiveflag = true;
	state=STATE_TX;
}


void lora_init(void)
{
	txNumber=0;
	Rssi=0;
	rxNumber = 0;
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxDone = OnRxDone;

	pinMode(LORA_PA_POWER,ANALOG);
	pinMode(LORA_PA_EN,OUTPUT);
	pinMode(LORA_PA_TX_EN,OUTPUT);
	digitalWrite(LORA_PA_EN,HIGH);
	digitalWrite(LORA_PA_TX_EN,HIGH);


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
	state=STATE_TX;
	factory_display.clear();
	packet ="waiting lora data!";
	factory_display.drawString(0, 10, packet);
	factory_display.display();
}


/********************************* lora  *********************************************/

void custom_delay(uint32_t time_ms)
{
#if 1
	uint32_t conut = time_ms/10;
	while (conut > 0)
	{
		conut --;
		delay(10);
		if(	interrupt_flag == true)
		{
			delay(200);
			if(digitalRead(0)==0)
			{
				break;
			}
		}
	}
#else
		delay(time_ms);
#endif
}

void wifi_connect_init(void)
{
	// Set WiFi to station mode and disconnect from an AP if it was previously connected
	WiFi.disconnect(true);
	custom_delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	WiFi.begin("Your WiFi SSID","Your Password");//fill in "Your WiFi SSID","Your Password"
	factory_display.drawString(0, 20, "WIFI Setup done");
	factory_display.display();
}

bool wifi_connect_try(uint8_t try_num)
{
	uint8_t count;
	while(WiFi.status() != WL_CONNECTED && count < try_num)
	{
		count ++;
		factory_display.clear();
		factory_display.drawString(0, 0, "wifi connecting...");
		factory_display.display();
		custom_delay(500);
	}
	if(WiFi.status() == WL_CONNECTED)
	{
		factory_display.clear();
		factory_display.drawString(0, 0, "wifi connect OK");
		factory_display.display();
		custom_delay(2500);
		return true;
	}
	else
	{
		factory_display.clear();
		factory_display.drawString(0, 0, "wifi connect failed");
		factory_display.display();
		custom_delay(1000);
		return false;
	}
	
}

void wifi_scan(unsigned int value)
{
	unsigned int i;
	WiFi.disconnect(); //
    WiFi.mode(WIFI_STA);
	factory_display.clear();
	for(i=0;i<value;i++)
	{
		factory_display.clear();
		factory_display.drawString(0, 0, "Scan start...");
		factory_display.display();
		int n = WiFi.scanNetworks();
		factory_display.drawString(0, 30, "Scan done");
		factory_display.display();

		if (n == 0)
		{
			factory_display.clear();
			factory_display.drawString(0, 0, "no network found");
			factory_display.display();
			custom_delay(2000);
		}
		else
		{
			factory_display.clear();
			factory_display.drawString(0, 0, (String)n);
			factory_display.drawString(20, 0, "networks found:");
			uint8_t index=1,i=0;
			String str = (String)(i + 1) +":"+(String)(WiFi.SSID(i))+" ("+(String)(WiFi.RSSI(i))+")";
			factory_display.drawString(0, index*10, str);
			i++;index++;
			str = (String)(i + 1) +":"+(String)(WiFi.SSID(i))+" ("+(String)(WiFi.RSSI(i))+")";
			factory_display.drawString(0, index*10, str);
			i++;index++;
			str = (String)(i + 1) +":"+(String)(WiFi.SSID(i))+" ("+(String)(WiFi.RSSI(i))+")";
			factory_display.drawString(0, index*10, str);
			i++;index++;
			str = (String)(i + 1) +":"+(String)(WiFi.SSID(i))+" ("+(String)(WiFi.RSSI(i))+")";
			factory_display.drawString(0, index*10, str);
			i++;index++;
			str = (String)(i + 1) +":"+(String)(WiFi.SSID(i))+" ("+(String)(WiFi.RSSI(i))+")";
			factory_display.drawString(0, index*10, str);
			factory_display.display();
			custom_delay(3000);
		}
	}
}

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
			if(rxNumber < 2)
			{
				delay(500);
				if(digitalRead(0)==0)
				{
					test_status = GPS_TEST;
				}
				else
				{
					resendflag=true;
				}
			}
			else
			{
				test_status = DEEPSLEEP_TEST;
				// deepsleepflag=true;
			}
		}
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


void enter_deepsleep(void)
{
	VextOFF();
	Radio.Sleep();
	SPI.end();
	pinMode(2,OUTPUT);
	digitalWrite(2,LOW);
	rtc_gpio_hold_en(GPIO_NUM_2);
	rtc_gpio_isolate(GPIO_NUM_2);
	pinMode(7,OUTPUT);
	digitalWrite(7,LOW);
	rtc_gpio_hold_en(GPIO_NUM_7);
	rtc_gpio_isolate(GPIO_NUM_7);
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

void lora_status_handle(void)
{
	if(resendflag)
	{
		state = STATE_TX;
		resendflag = false;
	}

	if(receiveflag && (state==LOWPOWER) )
	{
		receiveflag = false;
		packet ="Rdata:";
		int i = 0;
		while(i < rxSize)
		{
			packet += rxpacket[i];
			i++;
		}
		// packSize = "R_Size:";
		// packSize += String(rxSize,DEC);
		String packSize = "R_rssi:";
		packSize += String(Rssi,DEC);
		send_num = "send_num:";
		send_num += String(txNumber,DEC);

		factory_display.clear();
		factory_display.drawString(0, 0, packet);
		factory_display.drawString(0, 20, packSize);
		factory_display.drawString(0, 40, send_num);
		factory_display.display();
		delay(100);

		if((rxNumber%2)==0)
		{
			digitalWrite(LED, HIGH);  
		}
	}
	switch(state)
	{
		case STATE_TX:
			delay(1000);
			txNumber++;
			sprintf(txpacket,"hello %d,Rssi:%d",txNumber,Rssi);
			Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
			Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
			state=LOWPOWER;
			break;
		case STATE_RX:
			Serial.println("into RX mode");
			Radio.Rx( 0 );
			state=LOWPOWER;
			break;
		case LOWPOWER:
			Radio.IrqProcess( );
			break;
		default:
			break;
	}
}

void gps_test(void)
{
	uint32_t clear_num = 0;
	uint32_t last_second=0;
	pinMode(VGNSS_CTRL,OUTPUT);
	digitalWrite(VGNSS_CTRL,LOW);
	pinMode(42,OUTPUT);
	digitalWrite(42,HIGH);
	Serial1.begin(9600,SERIAL_8N1,39,38);    
	Serial.println("gps_test");
	factory_display.clear();
	delay(100);
	factory_display.drawString(0, 0, (String)"gps_test");
	factory_display.display();

	while(1)
	{
		if(Serial1.available()>0)
		{
			if(Serial1.peek()!='\n')
			{
				gps.encode(Serial1.read());
			}
			else
			{
				Serial1.read();
				factory_display.clear();
				factory_display.drawString(0, 0, (String)"gps_detected");
				factory_display.display();
				if(gps.time.second()==0)
				{
					continue;
				}
				String time_str = (String)gps.time.hour() + ":" + (String)gps.time.minute() + ":" + (String)gps.time.second()+ ":"+(String)gps.time.centisecond();
				factory_display.drawString(0, 15, time_str);
				String latitude = "LAT: " + (String)gps.location.lat();
				factory_display.drawString(0, 30, latitude);
				String longitude  = "LON: "+  (String)gps.location.lng();
				factory_display.drawString(0, 45, longitude);
				factory_display.display();

				Serial.printf(" %02d:%02d:%02d.%02d",gps.time.hour(),gps.time.minute(),gps.time.second(),gps.time.centisecond());
				Serial.print("LAT: ");
				Serial.print(gps.location.lat(),6);
				Serial.print(", LON: ");
				Serial.print(gps.location.lng(),6);
				Serial.println();
				if(last_second != gps.time.second())
				{
					last_second = gps.time.second();
					delay(1000);
					while(Serial1.read()>0);
				}
				else
				{
					delay(10);
					clear_num++;
					if(clear_num%5==0)
					{
						while(Serial1.read()>0);
					}
				}
			}
		}
	}
}

void logo(){
	factory_display.clear();
	factory_display.drawXbm(0,5,logo_width,logo_height,(const unsigned char *)logo_bits);
	factory_display.display();
}
void setup()
{
	rtc_gpio_hold_dis(GPIO_NUM_2);
	rtc_gpio_hold_dis(GPIO_NUM_7);
	Serial.begin(115200);
	Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);

	VextON();
	factory_display.init();
	factory_display.clear();
	factory_display.display();
	logo();
	delay(300);
	factory_display.clear();

	attachInterrupt(0,interrupt_GPIO0,FALLING);
	resendflag=false;
	deepsleepflag=false;
	interrupt_flag = false;

	chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
	Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
	Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
 
	pinMode(LED ,OUTPUT);
	digitalWrite(LED, LOW);
	test_status = WIFI_CONNECT_TEST_INIT;
}


void loop()
{
	interrupt_handle();
	switch (test_status)
	{
		case WIFI_CONNECT_TEST_INIT:
		{
			wifi_connect_init();
			test_status = WIFI_CONNECT_TEST;
		}
		case WIFI_CONNECT_TEST:
		{
			if(wifi_connect_try(2)==true)
			{
				test_status = WIFI_SCAN_TEST;
			}
			wifi_connect_try_num--;
			break;
		}
		case WIFI_SCAN_TEST:
		{
			wifi_scan(1);
			test_status = LORA_TEST_INIT;
			break;
		}
		case LORA_TEST_INIT:
		{
			lora_init();
			test_status = LORA_COMMUNICATION_TEST;
			break;
		}
		case LORA_COMMUNICATION_TEST:
		{
			lora_status_handle();
			break;
		}
		case DEEPSLEEP_TEST:
		{
			enter_deepsleep();
			break;
		}
		case GPS_TEST:
		{
			gps_test();
			break;
		}
		default:
			break;
	}
}