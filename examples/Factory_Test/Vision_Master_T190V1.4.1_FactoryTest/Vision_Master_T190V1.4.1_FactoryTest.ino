/* 
 * dependency libraries: "Adafruit_ST7789.h" and related lib
 * HelTec Automation(TM) Vision Master T190 factory test code, witch includ
 * follow functions: Adafruit_ST7789.h.
 * 
 * - Basic TFT-LCD function test;
 * 
 * - Basic serial port test(in baud rate 115200);
 * 
 * - WIFI connect and scan test;
 * 
 * - LoRa basic test;
 *
 * - I2C sensor interface function;
 *
 * - Joystick test;
 * 
 * - ADC test.
 *
 * - Timer test and some other Arduino basic functions;
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
#include "HT_TinyGPS++.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <driver/gpio.h>
#include <driver/rtc_io.h>
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
#define VGNSS_CTRL 18
#define TEST_WIFI_SSID       "Your WiFi SSID"          //Your Wi-Fi SSID
#define TEST_WIFI_PWD        "Your Password"           //Your Wi-Fi Password

#define Resolution 0.000244140625
#define battary_in 3.3
#define coefficient 5

typedef enum 
{
  SWITCH_TEST,
  GPS_TEST,
	WIFI_CONNECT_TEST,
	WIFI_SCAN_TEST,
	LORA_TEST_INIT,
	LORA_COMMUNICATION_TEST,
	DEEPSLEEP_TEST,
}test_status_t;

static SPIClass  *gspi_lcd  = NULL;//spi object pointer for lcd display
TinyGPSPlus gps;
test_status_t  test_status;
static Adafruit_ST7789  *factory_display = NULL;
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
int16_t red_x=155,red_y=80;
int wifiScanCnt=0;
int wifiScanMaxRssi=-255;
bool wifiConnected=false;
bool loratimeout=false;
bool Boot1_state = false;
float battery_levl;
// extern uint32_t ex_32k_start_cnt;

void spi_init(void){
 gspi_lcd = new SPIClass(HSPI);
 gspi_lcd->begin(38, -1, 48, 39);
 pinMode(gspi_lcd->pinSS(), OUTPUT);
}
void testdrawtext(uint16_t x, uint16_t y, char *text, uint16_t color,uint16_t size) {
  factory_display->setTextSize(size);
  factory_display->setCursor(x, y);
  factory_display->setTextColor(color);
  factory_display->setTextWrap(true);
  factory_display->print(text);
}
void OnTxDone( void )
{
	state=STATE_RX;
}

void OnTxTimeout( void )
{
  Radio.Sleep( );
	state=STATE_TX;
}

void battery() {
  analogReadResolution(12);
  battery_levl = analogRead(6) * Resolution * battary_in * coefficient;  //battary/4096*3.3* coefficient
}

void logo(){  
  factory_display->fillScreen(ST77XX_BLACK);
  factory_display->drawRGBBitmap(0, 18, heltec_logo, 319,133);
}

void showStatus(int value)
{
  factory_display->fillScreen(ST77XX_BLACK);
  packet ="WIFI   ";
  if(wifiConnected)
  {
    packet+="OK";
    testdrawtext(0,0,(char*)packet.c_str(),ST77XX_GREEN,2);
  }
  else
  {
    packet+="XX";
    testdrawtext(0,0,(char*)packet.c_str(),ST77XX_RED,2);
  }
  packet ="WIFI Scan Cnt:"+String(wifiScanCnt);
  testdrawtext(0,18,(char*)packet.c_str(),ST77XX_WHITE,2);
  packet ="Rssi:"+String(wifiScanMaxRssi);
  testdrawtext(0,36,(char*)packet.c_str(),ST77XX_WHITE,2);
  if(value)
  {
    packet ="LoRa Tx Rssi : "+String(maxTxRssi);
    testdrawtext(0,54,(char*)packet.c_str(),ST77XX_GREEN,2);
    packet ="LoRa Rx Rssi : "+String(maxRxRssi);
    testdrawtext(0,72,(char*)packet.c_str(),ST77XX_GREEN,2);
  }
  else
  {
    packet ="LoRa Error";
    testdrawtext(0,72,(char*)packet.c_str(),ST77XX_RED,2);
  }
  battery();
  packet = "ADC analog value = "+String(battery_levl);
  if(battery_levl > 3 && battery_levl < 5)
  {
    testdrawtext(0,90,(char*)packet.c_str(),ST77XX_BLUE,2);
  }
  else
  {
    testdrawtext(0,90,(char*)packet.c_str(),ST77XX_RED,2);
  }
  delay(100);
  Wire.begin(2, 1);
  byte error, address;
  int nDevices = 0;
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      nDevices++;
      packet = "I2C address: "+String(address);
      testdrawtext(0,108,(char*)packet.c_str(),ST77XX_CYAN,2);
    } 
    else if (error != 2) {
      packet = "Error  "+String(error)+" at address "+String(address);
      testdrawtext(0,75,(char*)packet.c_str(),ST77XX_RED,2);
    }
  }
  if(nDevices == 0) 
  {
    packet = "No I2C devices found!";
    testdrawtext(0,108,(char*)packet.c_str(),ST77XX_RED,2);
  }
  delay(1000);
  uint32_t t=millis()/1000;
  packet ="time:"+String(t);
  testdrawtext(190,0,(char*)packet.c_str(),ST77XX_WHITE,2);
  while (digitalRead(21)) {
    Boot1_state = true;
  }
  if (Boot1_state == true) {
    packet ="User Boot test is OK!";
    testdrawtext(0,126,(char*)packet.c_str(),ST77XX_WHITE,2);
    delay(500);
    logo();
  }
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  Radio.Sleep( );
  if(test_mode==0)
  {
    state=STATE_TX;
    if(size==10)
    {
      uint64_t rxchipid=*((uint64_t *)payload);
      if(rxchipid==chipid)
      {
        int16_t txrssi=*(int16_t *)(payload+8);
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
  }
}

void OnRxError()
{
  if(test_mode==0)
  {
    Radio.Sleep();
    state=STATE_TX;
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

void WIFISetUp(void)
{
	WiFi.disconnect(true);
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	WiFi.begin(TEST_WIFI_SSID,TEST_WIFI_PWD);//fill in "Your WiFi SSID","Your Password"
	delay(100);
  packet = "Connecting...";
  factory_display->fillScreen(ST77XX_BLACK);
  testdrawtext(0,0,(char*)packet.c_str(),ST77XX_WHITE,2);
	byte count = 0;
	while(WiFi.status() != WL_CONNECTED && count < 10)
	{
		count ++;
		delay(500);
	}
	if(WiFi.status() == WL_CONNECTED)
	{
    packet = "Connecting...OK";
    testdrawtext(0,0,(char*)packet.c_str(),ST77XX_WHITE,2);
    wifiConnected = true;
	}
	else
	{
    packet = "Connecting...Failed";
    testdrawtext(0,0,(char*)packet.c_str(),ST77XX_WHITE,2);
	}
  packet = "WIFI Setup done";
  testdrawtext(0,18,(char*)packet.c_str(),ST77XX_WHITE,2);
	delay(500);
}

void WIFIScan(unsigned int value)
{
	unsigned int i;
  WiFi.mode(WIFI_STA);
	for(i=0;i<value;i++)
	{
    packet = "Scan start...";
    testdrawtext(0,36,(char*)packet.c_str(),ST77XX_WHITE,2);
		int n = WiFi.scanNetworks();
    wifiScanCnt=n;
    packet = "Scan done";
    testdrawtext(0,54,(char*)packet.c_str(),ST77XX_WHITE,2);
		delay(500);
		if (n == 0)
		{
      packet = "no network found";
      testdrawtext(0,72,(char*)packet.c_str(),ST77XX_WHITE,2);
		}
		else
		{
      packet = String(n) + " networks found:";
      testdrawtext(0,72,(char*)packet.c_str(),ST77XX_WHITE,2);
			delay(500);
      factory_display->fillScreen(ST77XX_BLACK);
			for (int i = 0; i < n; ++i) {
      if(wifiScanMaxRssi<WiFi.RSSI(i))
        {
          wifiScanMaxRssi=WiFi.RSSI(i);
        }
        packet = String(i + 1) + ":" + String(WiFi.SSID(i)) + "(" + String(WiFi.RSSI(i)) + ")";
        testdrawtext(0,i * 18 + 8,(char*)packet.c_str(),ST77XX_GREEN,2);
				delay(10);
			}
		}
		delay(800);
	}
}


void VextON(void)
{
  pinMode(5,OUTPUT);
  digitalWrite(5, HIGH);
  pinMode(17,OUTPUT);
  digitalWrite(17, HIGH);
  pinMode(46,OUTPUT);
  digitalWrite(46,HIGH);
  pinMode(7,OUTPUT);
  digitalWrite(7,LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(5,OUTPUT);
  digitalWrite(5, LOW);
  pinMode(17,OUTPUT);
  digitalWrite(17, LOW);
  pinMode(46,OUTPUT);
  digitalWrite(46, LOW);
  pinMode(7,OUTPUT);
  digitalWrite(7, HIGH);
}

void intodeepsleep()
{
  VextOFF();
  Radio.Sleep();
  SPI.end();
  pinMode(RADIO_DIO_1,ANALOG);
  digitalWrite(RADIO_NSS, HIGH);
  rtc_gpio_hold_en(gpio_num_t(8));
 
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
  pinMode(21,INPUT);
  pinMode(16, INPUT);//CW
  pinMode(15, INPUT);//CCW
  while(1)
  {
    if(digitalRead(USERKEY)==0)
    {
      keydowntime=millis();
      delay(10);
      while(digitalRead(USERKEY)==0){
        if( (millis()-keydowntime)>3000 )
        {
          break;
        }
      }
      if( (millis()-keydowntime)>3000 )
      {
        test_status = LORA_TEST_INIT;
        test_mode=1;
        factory_display->fillScreen(ST77XX_BLACK);
        packet ="LORA MODE "+String(test_mode);
        testdrawtext(50,72,(char*)packet.c_str(),ST77XX_WHITE,2);
        delay(1000);
        state=STATE_RX;
      }
      else
      {
        intodeepsleep();
      }
    }

    if(digitalRead(21)==0) {
      if(test_status == SWITCH_TEST ) test_status = GPS_TEST;
      else if(test_status == GPS_TEST ) test_status = WIFI_CONNECT_TEST;
      delay(300);
    }

  }
}

void DIP_switch_test(void) {
  if(digitalRead(16)==0) {
    // red_x = red_x +20;
    if(red_x<= -5)  red_x = 315;
    else          red_x = red_x -20;
  }
  else if(digitalRead(15)==0) {
    if(red_x >= 315) red_x = -5;
    else          red_x = red_x +20;
  }
  // if(digitalRead(21)==0) {
  //   // if(red_y<= 0)  red_y = 160;
  //   // else           red_y = red_y -20;
  // }
  factory_display->fillScreen(ST77XX_BLACK);
  factory_display->fillRect(red_x,red_y,20,20,ST77XX_RED);
  delay(100);
}

void gps_test(void)
{
	uint32_t clear_num = 0;
	uint32_t last_second=0;
	pinMode(VGNSS_CTRL,OUTPUT);
	digitalWrite(VGNSS_CTRL,LOW);
	pinMode(42,OUTPUT);
	digitalWrite(42,HIGH);
	Serial1.begin(9600,SERIAL_8N1,43,44);
	Serial.println("gps_test");
  factory_display->fillScreen(ST77XX_BLACK);
	delay(100);
  packet ="gps_test";
  testdrawtext(0,0,(char*)packet.c_str(),ST77XX_WHITE,2);

	while(1)
	{
    if(test_status == WIFI_CONNECT_TEST || test_status == LORA_TEST_INIT) {
      break;
    }
		if(Serial1.available()>0)
		{
			if(Serial1.peek()!='\n')
			{
				gps.encode(Serial1.read());
			}
			else
			{
				Serial1.read();
				// factory_display.clear();
				// factory_display.drawString(0, 0, (String)"gps_detected");
				// factory_display.display();
        packet ="gps_test: gps_detected";
        testdrawtext(0,0,(char*)packet.c_str(),ST77XX_WHITE,2);
				if(gps.time.second()==0)
				{
					continue;
				}
				String time_str = (String)gps.time.hour() + ":" + (String)gps.time.minute() + ":" + (String)gps.time.second()+ ":"+(String)gps.time.centisecond();
				// factory_display.drawString(0, 15, time_str);
        testdrawtext(0,15,(char*)time_str.c_str(),ST77XX_WHITE,2);
				String latitude = "LAT: " + (String)gps.location.lat();
				// factory_display.drawString(0, 30, latitude);
        testdrawtext(0,30,(char*)latitude.c_str(),ST77XX_WHITE,2);
				String longitude  = "LON: "+  (String)gps.location.lng();
				// factory_display.drawString(0, 45, longitude);
        testdrawtext(0,45,(char*)longitude.c_str(),ST77XX_WHITE,2);
				// factory_display.display();

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

void lora_test(void)
{
  if(test_mode)
  {
    switch(state)
    {
      case STATE_TX:
        Radio.SetChannel( RF_FREQUENCY_2 );
        Radio.Send( txpacket, 10 );
        state=LOWPOWER;
        break;
      case STATE_RX:
        Radio.SetChannel( RF_FREQUENCY_1 );
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
        Radio.Send( (uint8_t *)&chipid, 8 );
        state=LOWPOWER;
        break;
      case STATE_RX:
        Radio.SetChannel( RF_FREQUENCY_2 );
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
  Radio.IrqProcess();
}

extern void checklicense();

void setup()
{
  Serial.begin(115200);
  checklicense();
  spi_init();
  factory_display = new Adafruit_ST7789(gspi_lcd, 39, 47, 40);
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\r\n",(uint32_t)chipid);//print Low 4bytes.
  xTaskCreateUniversal(checkUserkey, "checkUserkey1Task", 2048, NULL, 1, &checkUserkey1kHandle, CONFIG_ARDUINO_RUNNING_CORE);
  delay(100);
  VextON();
  factory_display->init(170, 320);
  factory_display->setRotation(1);
  factory_display->fillScreen(ST77XX_GREEN);
  
  test_status = SWITCH_TEST;
  delay(1000);
}

void loop()
{
  switch (test_status)
	{
		case SWITCH_TEST:
		{
			DIP_switch_test();
      break;
		}
    case GPS_TEST:
		{
			gps_test();
      break;
		}
    case WIFI_CONNECT_TEST:
		{
			WIFISetUp();
			test_status = WIFI_SCAN_TEST;
		}
		case WIFI_SCAN_TEST:
		{
			WIFIScan(1);
			test_status = LORA_TEST_INIT;
			break;
		}
		case LORA_TEST_INIT:
		{
			lora_init();
      packet ="LORA MODE "+String(test_mode);
      factory_display->fillScreen(ST77XX_BLACK);
      testdrawtext(50,72,(char*)packet.c_str(),ST77XX_WHITE,3);
			test_status = LORA_COMMUNICATION_TEST;
			break;
		}
    case LORA_COMMUNICATION_TEST:
		{
      lora_test();
			break;
		}
  }
}