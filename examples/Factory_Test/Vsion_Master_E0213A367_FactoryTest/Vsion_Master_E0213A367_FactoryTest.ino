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
#include "HT_lCMEN2R13EFC1.h"
#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <HardwareSerial.h>
#include "HT_E0213A367.h"
#include "driver/board-config.h"
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

#define Resolution 0.000244140625
#define battary_in 3.3
#define coefficient 4.01
#define LED  45
#define DATA_SIZE 32    
#define BAUD 9600       
#define RXPIN 43         
#define TXPIN 44    
uint8_t TX_send[DATA_SIZE] = {0x48,0x65,0x6C,0x6C,0x6F,0x2C,0x57,0x6F,0x72,0x6C,0x64,0x21,0x0D,0x0A};
uint8_t Test_send[DATA_SIZE] = {0x55,0x61,0x72,0x74,0x20,0x74,0x65,0x73,0x74,0x20,0x69,0x73,0x20,0x4F,0x4B,0x21,0x0D,0x0A};
uint8_t Uart_test_state = 0;
uint8_t RX_buff[DATA_SIZE];
uint8_t rx_num = 0;

ScreenDisplay *factory_display;
// HT_E0213A367   factory_display(3, 2, 5, 1, 4, 6, -1, 6000000);
// HT_ICMEN2R13EFC1 factory_display(3, 2, 5, 1, 4, 6, -1, 6000000); // rst,dc,cs,busy,sck,mosi,miso,frequency

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
float battery_levl;
extern uint32_t ex_32k_start_cnt;
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
  battery_levl = analogRead(7) * Resolution * battary_in * coefficient;  //battary/4096*3.3* coefficient
}

void uart0_int(void) {
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN);
  Serial1.write(TX_send, DATA_SIZE);
}

void uart0_test(void) {
    if(Uart_test_state == 1)
    {
      Uart_test_state = 0;
      Serial1.write(Test_send, DATA_SIZE);
      pinMode(LED,OUTPUT);
      digitalWrite(LED, LOW);
    }
    if(rx_num > 0)
    {
      Serial1.write(RX_buff,rx_num);
      rx_num = 0;
      Uart_test_state = 1;
    }
    while (Serial1.available() > 0 )
    {
      RX_buff[rx_num] = Serial1.read();
      rx_num++;
    }
    delay(50);
}


void logo(){
	factory_display->clear();
	factory_display->drawXbm(50 , 35 , logo_width, logo_height, logo_bits);
  factory_display->update(BLACK_BUFFER);
	factory_display->display();
}

void showStatus(int value)
{
  int a = 0,b = 0,c = 0;
  factory_display->clear();
  factory_display->setFont(ArialMT_Plain_10);
  factory_display->setTextAlignment(TEXT_ALIGN_LEFT);
  packet ="WIFI   ";
  if(wifiConnected)
  {
    packet+="OK";
  }
  else
  {
    packet+="XX";
  }
  factory_display->drawString(0, 0, packet);
  packet ="WIFI Scan Cnt:"+String(wifiScanCnt)+" Rssi:"+String(wifiScanMaxRssi);
  factory_display->drawString(0, 15, packet);
  if(value)
  {
    a = abs(maxTxRssi);
    b = abs(maxRxRssi);
    c = abs(a - b);
    packet ="LoRa Tx Rssi : "+String(maxTxRssi);
    factory_display->drawString(0, 30, packet);
    packet ="LoRa Rx Rssi : "+String(maxRxRssi);
    factory_display->drawString(0, 45, packet);
    if(c > 20)
    {
      packet ="LoRa Test XX";
      factory_display->drawString(0, 75, packet);
    }
  }
  else
  {
    packet ="LoRa Error";
    factory_display->drawString(0, 45, packet);
  }
  battery();
  packet = "ADC analog value = "+String(battery_levl);
  factory_display->drawString(0, 60, packet);
  delay(100);
  Wire.begin(39, 38);
  byte error, address;
  int nDevices = 0;
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      packet = "I2C device found at address: "+String(address);
      factory_display->drawString(0, 75, packet);
      nDevices++;
    } else if (error != 2) {
      packet = "Error  "+String(error)+" at address "+String(address);
      factory_display->drawString(0, 75, packet);
    }
  }
  if (nDevices == 0) {
    packet = "No I2C devices found!";
    factory_display->drawString(0, 75, packet);
  }
  delay(1000);
  uint32_t t=millis()/1000;
  factory_display->setTextAlignment(TEXT_ALIGN_RIGHT);
  packet ="time:"+String(t);
  factory_display->drawString(128, 0, packet);
  factory_display->update(BLACK_BUFFER);
  factory_display->display();
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
	byte count = 0;
	while(WiFi.status() != WL_CONNECTED && count < 10)
	{
		count ++;
		delay(500);
	}
	if(WiFi.status() == WL_CONNECTED)
	{
    wifiConnected = true;
	}
  else
  {
    
  }
	delay(500);
}

void WIFIScan(unsigned int value)
{
	unsigned int i;
  WiFi.mode(WIFI_STA);
	for(i=0;i<value;i++)
	{
		int n = WiFi.scanNetworks();
    wifiScanCnt=n;
		delay(500);
		if(n != 0)
		{
			delay(500);
			for (int i = 0; i < n; ++i) {
      if(wifiScanMaxRssi<WiFi.RSSI(i))
        {
          wifiScanMaxRssi=WiFi.RSSI(i);
        }
			}
		}
		delay(500);
	}
  pinMode(LED,OUTPUT);
  digitalWrite(LED, HIGH);
}

void VextON(void)
{
  pinMode(18,OUTPUT);
  digitalWrite(18, HIGH);
  pinMode(46,OUTPUT);
  digitalWrite(46, HIGH);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(18,OUTPUT);
  digitalWrite(18, LOW);
  pinMode(46,OUTPUT);
  digitalWrite(46, LOW);
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
  while(1)
  {
    if(digitalRead(21) == 0)
    {
      logo();
      SPI.end();
      while(1)
      {
        uart0_test();
      }
    }
    else if(digitalRead(USERKEY)==0)
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
        test_mode=1;
        factory_display->clear();
        factory_display->setFont(ArialMT_Plain_24);
        factory_display->setTextAlignment(TEXT_ALIGN_LEFT);
        packet ="LORA MODE "+String(test_mode);
        factory_display->drawString(40, 48, packet);
        factory_display->update(BLACK_BUFFER);
        factory_display->display();
        delay(1000);
        factory_display->setFont(ArialMT_Plain_10);
        factory_display->setTextAlignment(TEXT_ALIGN_LEFT);
        state=STATE_RX;
      }
      else
      {
        intodeepsleep();
      }
    }
  }
}
extern void checklicense();

#define PIN_EINK_SCLK 4
#define PIN_EINK_DC   2
#define PIN_EINK_CS   5
#define PIN_EINK_RES  3
#define PIN_EINK_MOSI 6

void setup()
{
  Serial.begin(115200);
  checklicense();
  delay(100);
  VextON();
  delay(100);
  pinMode(PIN_EINK_SCLK, OUTPUT); 
    pinMode(PIN_EINK_DC, OUTPUT); 
    pinMode(PIN_EINK_CS, OUTPUT);
    pinMode(PIN_EINK_RES, OUTPUT);
    
    //rest e-ink
    digitalWrite(PIN_EINK_RES, LOW);
    delay(20);
    digitalWrite(PIN_EINK_RES, HIGH);
    delay(20);

    digitalWrite(PIN_EINK_DC, LOW);
    digitalWrite(PIN_EINK_CS, LOW);

    // write cmd
    uint8_t cmd = 0x2F;
    pinMode(PIN_EINK_MOSI, OUTPUT);  
    digitalWrite(PIN_EINK_SCLK, LOW);
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(PIN_EINK_MOSI, (cmd & 0x80) ? HIGH : LOW);
        cmd <<= 1;
        digitalWrite(PIN_EINK_SCLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(PIN_EINK_SCLK, LOW);
        delayMicroseconds(1);
    }
    delay(10);

    digitalWrite(PIN_EINK_DC, HIGH);
    pinMode(PIN_EINK_MOSI, INPUT_PULLUP); 

    // read chip ID
    uint8_t chipId = 0;
    for (int8_t b = 7; b >= 0; b--) 
    {
      digitalWrite(PIN_EINK_SCLK, LOW);  
      delayMicroseconds(1);
      digitalWrite(PIN_EINK_SCLK, HIGH);
      delayMicroseconds(1);
      if (digitalRead(PIN_EINK_MOSI)) chipId |= (1 << b);  
    }
    digitalWrite(PIN_EINK_CS, HIGH);
    if((chipId &0x03) !=0x01)
    {
      factory_display = new HT_ICMEN2R13EFC1(3, 2, 5, 1, 4, 6, -1, 6000000); // rst,dc,cs,busy,sck,mosi,miso,frequency
    } 
    else
    {
      factory_display = new HT_E0213A367(3, 2, 5, 1, 4, 6, -1, 6000000); // rst,dc,cs,busy,sck,mosi,miso,frequency
    }



  if(ex_32k_start_cnt>5)
  {
    VextON();
    delay(100);
    factory_display->init();
    factory_display->setFont(ArialMT_Plain_24);
    factory_display->setTextAlignment(TEXT_ALIGN_LEFT);
    packet ="EX 32K ERROR";
    factory_display->clear();
    factory_display->drawString(40, 48, packet);
    factory_display->update(BLACK_BUFFER);
    factory_display->display();
    while(1);
  }
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\r\n",(uint32_t)chipid);//print Low 4bytes.
  xTaskCreateUniversal(checkUserkey, "checkUserkey1Task", 2048, NULL, 1, &checkUserkey1kHandle, CONFIG_ARDUINO_RUNNING_CORE);
  delay(500);
  VextON();
  factory_display->init();
  delay(100);
  uart0_int();
  delay(100);
  lora_init();
	WIFISetUp();
	delay(100);
	WIFIScan(1);
  factory_display->setFont(ArialMT_Plain_24);
  factory_display->setTextAlignment(TEXT_ALIGN_LEFT);
	packet ="LORA MODE "+String(test_mode);
  factory_display->clear();
  factory_display->drawString(40, 48, packet);
  packet ="HF";
  factory_display->drawString(210, 0, packet);
  factory_display->update(BLACK_BUFFER);
  factory_display->display();
  delay(100);
  factory_display->clear();
}

void loop()
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
  Radio.IrqProcess( );
}
