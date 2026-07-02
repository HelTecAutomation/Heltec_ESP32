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
#include "HT_TinyGPS++.h"
#include "driver/rtc_io.h"
#include "GXHTC.h"
#include <Adafruit_BME280.h>    
#include "da217.h"
#define SDFAT_FILE_TYPE 3
#include <SdFat.h>

static const uint8_t SD_CS_PIN   = 3;  // P1.00
static const uint8_t SD_MOSI_PIN = 15;  // P1.01
static const uint8_t SD_SCK_PIN  = 16;   // P0.06
static const uint8_t SD_MISO_PIN = 45;  // P0.26

static const uint32_t SERIAL_BAUD = 115200;
static const char *RW_TEST_FILE_PATH = "/sd_rw_test.txt";
static const char *RW_TEST_CONTENT =
    "MeshTower SD read/write test OK.\r\n"
    "1234567890\r\n";

SdFs sd;
FsFile root;
FsFile entry;


#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER1                            22        // 20 dBm
#define TX_TIMEOUT                                  10        // seconds (MAX value)


/********************************* lora  *********************************************/
#define RF_FREQUENCY_1                              868500000 // Hz
#define RF_FREQUENCY_2                              870500000 // Hz

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
#define BATTER_MIN 3800
#define BATTER_MAX 4300
#define AVERAGE_CNT 10
#define beep_pin  4
static const uint32_t BUZZER_FREQUENCY_HZ = 2000;
static const uint8_t BUZZER_LEDC_RESOLUTION = 8;
static const uint8_t BUZZER_DUTY = 128;
static const uint8_t BUZZER_BEEP_COUNT = 4;
static const uint16_t BUZZER_ON_MS = 250;
static const uint16_t BUZZER_OFF_MS = 250;
// #define LED 46
#define KEY 46

TinyGPSPlus gps;
GXHTC gxhtc1;
Adafruit_BME280 bme;
DA217 da217;

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
String packet,packet2,packet3,packet4;
String packet1 ;


bool receiveflag = false; // software flag for LoRa receiver, received data makes it true.
uint64_t chipid;
uint8_t rx_cnt=0;
int test_mode=0;
int wifiScanCnt=0;
int wifiScanMaxRssi=-255;
bool wifiConnected=false;
bool loratimeout=false;
bool deepsleep_state=false;
bool I2C_state=false;
uint16_t volt;
uint8_t err_count = 0;
bool gps_state=false;
bool gps_test_state=false;
uint32_t gps_get_time=0;
uint32_t gps_start_time=0;
uint8_t I2C_Addr[6];
uint8_t addrcount = 0;
uint8_t j = 0,k = 0;



void OnRadioTxTimeout( void )
{
    // Restarts continuous wave transmission when timeout expires
    Radio.SetTxContinuousWave( RF_FREQUENCY, TX_OUTPUT_POWER1, TX_TIMEOUT );
}

void printDivider() {
  Serial.println(F("--------------------------------------------------"));
}

void listRootDirectory() {
  if (!root.open("/")) {
    Serial.println(F("Failed to open root directory."));
    return;
  }

  Serial.println(F("Root directory:"));

  while (entry.openNext(&root, O_RDONLY)) {
    char name[80] = {0};
    entry.getName(name, sizeof(name));

    if (entry.isDir()) {
      Serial.print(F("[DIR ] "));
      Serial.println(name);
    } else {
      Serial.print(F("[FILE] "));
      Serial.print(name);
      Serial.print(F("  size="));
      entry.printFileSize(&Serial);
      Serial.println(F(" bytes"));
    }

    entry.close();
  }

  root.close();
}

bool writeReadVerifyTest(const char *path) {
  FsFile file;
  char readback[128] = {0};
  int index = 0;

  Serial.print(F("Writing test file: "));
  Serial.println(path);

  if (!file.open(path, O_RDWR | O_CREAT | O_TRUNC)) {
    Serial.println(F("Failed to create test file."));
    return false;
  }

  size_t expected_len = strlen(RW_TEST_CONTENT);
  size_t written = file.print(RW_TEST_CONTENT);
  file.close();

  if (written != expected_len) {
    Serial.print(F("Write length mismatch. expected="));
    Serial.print(expected_len);
    Serial.print(F(" actual="));
    Serial.println(written);
    return false;
  }

  Serial.println(F("Write test passed."));
  Serial.print(F("Reading back test file: "));
  Serial.println(path);

  if (!file.open(path, O_RDONLY)) {
    Serial.println(F("Failed to reopen test file."));
    return false;
  }

  while (file.available() && index < (int)sizeof(readback) - 1) {
    readback[index++] = (char)file.read();
  }
  readback[index] = '\0';
  file.close();

  printDivider();
  Serial.print(readback);
  if (index == 0 || readback[index - 1] != '\n') {
    Serial.println();
  }
  printDivider();

  if (strcmp(readback, RW_TEST_CONTENT) != 0) {
    Serial.println(F("Readback verification failed."));
    return false;
  }

  Serial.println(F("Readback verification passed."));
  return true;
}

void printCardInfo() {
  uint64_t sector_count = sd.card()->sectorCount();
  uint64_t card_size_bytes = sector_count * 512ULL;
  uint64_t card_size_mb = card_size_bytes / 1000000ULL;
  uint64_t card_size_gb_x100 = card_size_bytes / 10000000ULL;
  uint8_t fat_type = sd.fatType();

  Serial.print(F("Card size: "));
  Serial.print((uint32_t)card_size_mb);
  Serial.print(F(" MB / "));
  Serial.print((uint32_t)(card_size_gb_x100 / 100));
  Serial.print(F("."));
  uint8_t gb_fraction = card_size_gb_x100 % 100;
  if (gb_fraction < 10) {
    Serial.print(F("0"));
  }
  Serial.print(gb_fraction);
  Serial.println(F(" GB"));
  Serial.print(F("Volume type: "));
  if (fat_type == 16) {
    Serial.println(F("FAT16"));
  } else if (fat_type == 32) {
    Serial.println(F("FAT32"));
  } else if (fat_type > 32) {
    Serial.println(F("exFAT"));
  } else {
    Serial.println(F("unknown"));
  }
}
 
void SD_CARD_TEST(void)
{
  bool init_ok = false;
  bool list_ok = false;
  bool rw_ok = false;

  // waitForSerial();

  Serial.println();
  Serial.println(F("MeshTower SD card read/write test"));
  printDivider();
  Serial.println(F("Schematic pin map:"));
  Serial.println(F("  CS   = 3"));
  Serial.println(F("  MOSI = 15"));
  Serial.println(F("  SCK  = 16"));
  Serial.println(F("  MISO = 45"));
  printDivider();

  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);

  SPI.end();
  SPI.begin(SD_SCK_PIN,SD_MISO_PIN,  SD_MOSI_PIN);
  // SPI.begin();

  Serial.println(F("Initializing SD card..."));
  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(8))) {
    Serial.println(F("sd.begin() failed."));
    Serial.println(F("Please check:"));
    Serial.println(F("  1. Card is inserted"));
    Serial.println(F("  2. Card is formatted as FAT16/FAT32/exFAT"));
    Serial.println(F("  3. Wiring matches the schematic"));
    Serial.println(F("  4. SdFat library is installed"));
    printDivider();
    Serial.println(F("Overall test result: FAIL"));
    while (1) {
      delay(1000);
    }
  }

  init_ok = true;
  Serial.println(F("SD card init OK"));
  printCardInfo();
  printDivider();

  listRootDirectory();
  list_ok = true;
  printDivider();

  rw_ok = writeReadVerifyTest(RW_TEST_FILE_PATH);
  printDivider();

  Serial.print(F("Overall test result: "));
  if (init_ok && list_ok && rw_ok) {
    Serial.println(F("PASS"));
  } else {
    Serial.println(F("FAIL"));
  }
}


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

void Sensor_DA217(void)
{
  int16_t x_data, y_data, z_data;
  float x=0,y=0,z=0;
  delay(100);
  // da217.da217_init(SENSOR_SDA, SENSOR_SCL);
  da217.da217_init();
  da217.da217_set_odr_rate(DA217_ODR_500HZ);
  da217.da217_set_fifo_mode(FIFO_MODE_BYPASS);
  da217.da217_set_full_scale(FS_FULL_SCALE_16g);
  da217.da217_set_resolution(RESOLUTION_14_BIT);
  da217.da217_start_xyz_axis(true, true, true);
  delay(100);
  for(int i=0;i<AVERAGE_CNT;i++)
  {
    da217.da217_read_xyz_data(&x_data, &y_data, &z_data);
    x+=(x_data*16.0/8192);
    y+=(y_data*16.0/8192);
    z+=(z_data*16.0/8192);
    delay(10);
  }
  x/=AVERAGE_CNT;
  y/=AVERAGE_CNT;
  z/=AVERAGE_CNT;
  float t=sqrt(x*x+y*y+z*z);
  Serial.println();
  Serial.printf("DA217 : %f %f %f %f\r\n",x,y,z,t);
  packet = "X:" + String(x) + "         Y:" + String(y) ; 
  factory_display.drawString(0, 36, packet);
  packet = "Z:" + String(z) + "         T:" + String(t);
  factory_display.drawString(0, 48, packet);
  factory_display.display();
}


void Sensor_GXHTC(void)
{
  gxhtc1.begin(17,18);
  gxhtc1.read_data();
  Serial.print("Temperature:");
  Serial.print(gxhtc1.g_temperature);
  Serial.print("  Humidity:");
  Serial.println(gxhtc1.g_humidity);
  Serial.printf("id = %X\r\n",gxhtc1.read_id());
  packet = "T:" + String(gxhtc1.g_temperature) + "*C  H:" + String(gxhtc1.g_humidity) + "%";
  if(addrcount > 3)
  {
    factory_display.drawString(0, j  * 12, packet);
  }
  else
  {
    factory_display.drawString(0, (j+1) * 12, packet);
  }
  
}
void Sensor_BME280(void)
{ 
  bool status;
  status = bme.begin(0x76);  
  if (!status) {
    while (1);  
  }
  float temperature = bme.readTemperature();  
  float humidity = bme.readHumidity();       
  float pressure = bme.readPressure() / 100.0F;  
  float altitude = bme.readAltitude(1013.25F);  
  
  packet = "T:" + String(temperature) + "*C  P:" + String(pressure) + "Pa";
  if(addrcount > 3)
  {
    factory_display.drawString(0, j * 12, packet);
  }
  else
  {
    factory_display.drawString(0, (j+1) * 12, packet);
  }
  packet = "H:" + String(humidity) + "%   A:" + String(altitude) + "m";
  if(addrcount > 3)
  {
    factory_display.drawString(0, (j+1)  * 12, packet);
  }
  else
  {
    factory_display.drawString(0, (j + 2)  * 12, packet);
  }

  Serial.print(temperature);
  Serial.print(" *C\t");  

  Serial.print(humidity);
  Serial.print(" %RH\t");

  Serial.print(pressure);
  Serial.print(" hPa\t");

  Serial.print(altitude);
  Serial.println(" m");
}
void Buzzer_test(void)
{ 
  pinMode(beep_pin, OUTPUT);
  ledcAttach(beep_pin, BUZZER_FREQUENCY_HZ, BUZZER_LEDC_RESOLUTION);
  for(uint8_t i = 0; i < BUZZER_BEEP_COUNT; i++)
  {
    ledcWrite(beep_pin, BUZZER_DUTY);
    delay(BUZZER_ON_MS);
    ledcWrite(beep_pin, 0);
    delay(BUZZER_OFF_MS);
  }
  ledcWrite(beep_pin, 0);
  ledcDetach(beep_pin);
  digitalWrite(beep_pin, LOW);
}
void I2C_SCAN(void)
{
  Wire1.begin(17, 18);
  byte error, address;
  int nDevices = 0;
  delay(100);
  Serial.println("Scanning for I2C devices ...");
  for(address = 0x01; address < 0x7f; address++){
    Wire1.beginTransmission(address);
    error = Wire1.endTransmission();
    if (error == 0){
      Serial.printf("I2C device found at address 0x%02X\n", address);
      I2C_Addr[addrcount] = address;
      packet1 += String(address) + "  ";
      nDevices++;
      addrcount++;
    } else if(error != 2){
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0){
    Serial.println("No I2C devices found");
  }
}

void showStatus(int value)
{
  int a = 0,b = 0,c = 0;
  Serial.printf("WiFi %d %d %d\r\n",wifiConnected,wifiScanCnt,wifiScanMaxRssi);
  Serial.printf("LoRa %d %d\r\n",maxTxRssi,maxRxRssi);
  factory_display.setFont(ArialMT_Plain_10);
  factory_display.setTextAlignment(TEXT_ALIGN_LEFT);
  packet4 ="WIFI  ";
  if(wifiConnected)
  {
    packet4+="OK";
  }
  else
  {
    packet4+="XX";
    err_count++;
  }
  packet2 ="WIFI Scan Cnt:"+String(wifiScanCnt)+" Rssi:"+String(wifiScanMaxRssi);
  if(value)
  {
    a = abs(maxTxRssi);
    b = abs(maxRxRssi);
    c = abs(a - b);
    if(c > 20)
    {
      packet3 ="LoRa Rssi: " + String(maxTxRssi) + "  " +String(maxRxRssi) + "  XX";
      err_count++;
    }
    else
    {
      packet3 ="LoRa Rssi: " + String(maxTxRssi) + "  " +String(maxRxRssi) + "  OK";
    }
  }
  else
  {
    packet3 ="LoRa Error";
    err_count++;
  }
  uint32_t t=millis()/1000;
  packet4 +="             time:"+String(t);
  if(err_count == 0)
  {
    // digitalWrite(LED, HIGH);
    while(1)  
    {
      factory_display.clear();
      factory_display.drawString(0, 0, packet4);
      factory_display.drawString(0, 12, packet2);
      factory_display.drawString(0, 24, packet3);
      // Sensor_DA217();
      factory_display.display();
      delay(1000);
    }
  }
  else
  {
    while(1)
    {
      factory_display.clear();
      factory_display.drawString(0, 0, packet4);
      factory_display.drawString(0, 12, packet2);
      factory_display.drawString(0, 24, packet3);
      // Sensor_DA217();
      factory_display.display();
      // digitalWrite(LED, HIGH);
      // delay(500);
      // digitalWrite(LED, LOW);
      // delay(500);
    }
  }
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  Radio.Sleep( );
  Serial.println("RX done......");
  Serial.printf("Rx size : %d , rssi : %d, snr : %d\r\n",size,rssi,snr);
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
          if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
          {
            showStatus(1);
          }
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
  if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
  {
    factory_display.display();
  }
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
		factory_display.drawString(0, 0, "Connecting...");
		if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
    {
      factory_display.display();
    }
	}
	factory_display.clear();
	if(WiFi.status() == WL_CONNECTED)
	{
    wifiConnected=true;
		factory_display.drawString(0, 0, "Connecting...OK.");
		if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
    {
      factory_display.display();
    }
	}
	else
	{	
    factory_display.clear();
    factory_display.drawString(0, 0, "Connecting...Failed");
    if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
    {
      factory_display.display();
    }
	}
	factory_display.drawString(0, 10, "WIFI Setup done");
	if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
  {
    factory_display.display();
  }
	delay(500);
}

void WIFIScan(unsigned int value)
{
	unsigned int i;
  WiFi.mode(WIFI_STA);

	for(i=0;i<value;i++)
	{
		factory_display.drawString(0, 20, "Scan start...");
		if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
    {
      factory_display.display();
    }

		int n = WiFi.scanNetworks();
    wifiScanCnt=n;
		factory_display.drawString(0, 30, "Scan done");
		if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
    {
      factory_display.display();
    }
		delay(500);
		factory_display.clear();

		if (n == 0)
		{
			factory_display.clear();
			factory_display.drawString(0, 0, "no network found");
			if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
      {
        factory_display.display();
      }
      err_count++;
		}
		else
		{
			factory_display.drawString(0, 0, (String)n);
			factory_display.drawString(14, 0, "networks found:");
			if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
      {
        factory_display.display();
      }
			delay(500);

			for (int i = 0; i < n; ++i) {
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
				delay(10);
			}
		}
		if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
    {
      factory_display.display();
    }
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


void gps_test(void)
{
	uint32_t clear_num = 0;
	uint32_t last_second=0;
	pinMode(42,OUTPUT);
	digitalWrite(42,LOW);
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
				char c = Serial1.read();
				Serial.write(c);
				gps.encode(c);
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





void intodeepsleep()
{
  Serial.println("into deep sleep");
  VextOFF();
	Radio.Sleep();
	SPI.end();
	// pinMode(2,OUTPUT);
	// digitalWrite(2,LOW);
	// rtc_gpio_hold_en(GPIO_NUM_2);
	// rtc_gpio_isolate(GPIO_NUM_2);
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
TaskHandle_t checkUserkey1kHandle = NULL;
TaskHandle_t checkUserkeykHandle = NULL;

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
        if( (millis()-keydowntime)>2000 )
        {
          break;
        }
      }
      if( (millis()-keydowntime)>2000 )
      {
        gps_test_state=true;
        gps_test();
      }
      else
      {
        I2C_state=true;
        Serial.print(addrcount);
        if(addrcount > 0)
        {
          pinMode(5,OUTPUT);
          digitalWrite(5,HIGH);
          pinMode(6,OUTPUT);
          digitalWrite(6,HIGH);
          while(1)
          {
            factory_display.clear();
            factory_display.drawString(0, 0, packet1);
            for(j = 0;j < addrcount;j++)
            {
              if(I2C_Addr[j] == 112)
              {
                Sensor_GXHTC();
              }
              else if(I2C_Addr[j] == 118)
              {
                Sensor_BME280();
              }
            }
            factory_display.display();
            
            delay(1000);
          }
        }
      }
    }
  }
}
void checkUserkey1(void *pvParameters)
{
  uint32_t keydowntime1;
  pinMode(KEY,INPUT);
  while(1)
  {
    if(digitalRead(KEY)==1)
    {
      keydowntime1=millis();
      Serial.printf("key1 down : %u\r\n",keydowntime1);
      delay(10);
      while(digitalRead(KEY)==1){
        if( (millis()-keydowntime1)>2000 )
        {
          break;
        }
      }
      if( (millis()-keydowntime1)>2000 )
      {
        // gps_test_state=true;
        // gps_test();
      }
      else
      {
        SD_CARD_TEST();
      }
    }
  }
}

uint8_t sp_a = 28,sp_b = 0;
void setup()
{
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\r\n",(uint32_t)chipid);//print Low 4bytes.
  // pinMode(LED ,OUTPUT);
	// digitalWrite(LED, HIGH);  
  // pinMode(2 ,OUTPUT);
	// digitalWrite(2, HIGH); 
  // pinMode(15 ,OUTPUT);
	// digitalWrite(15, HIGH); 
  // pinMode(16 ,OUTPUT);
	// digitalWrite(16, HIGH); 
  RadioEvents.TxTimeout = OnRadioTxTimeout;
  Radio.Init( &RadioEvents );
  for(sp_b = 0;sp_b < 3;sp_b++)
  {
    Radio.IrqProcess( );
    Radio.SetTxContinuousWave( RF_FREQUENCY, sp_a, TX_TIMEOUT );
    sp_a += 2;
    if(sp_a > 32)
    {
      sp_a = 28;
    }
    delay(250);
    Radio.Sleep();
    delay(250);
  }
  VextON();
  Buzzer_test();
  // SD_CARD_TEST();
  xTaskCreateUniversal(checkUserkey, "checkUserkeyTask", 2048, NULL, 1, &checkUserkeykHandle, CONFIG_ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(checkUserkey1, "checkUserkey1Task", 2048, NULL, 1, &checkUserkey1kHandle, CONFIG_ARDUINO_RUNNING_CORE);
  // pinMode(LED ,OUTPUT);
	// digitalWrite(LED, LOW);
  delay(100);
  I2C_SCAN();
  delay(100);
  factory_display.init();
  factory_display.fillRect(0, 0, factory_display.getWidth(), factory_display.getHeight());
  factory_display.display();
  packet ="RF_FREQUENCY:HF";
  factory_display.clear();
  factory_display.drawString(0, 24, packet);
  if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
  {
    factory_display.display();
  }
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
  if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
  {
    factory_display.setFont(ArialMT_Plain_16);
    factory_display.setTextAlignment(TEXT_ALIGN_CENTER);
    packet ="LORA MODE "+String(test_mode);
    factory_display.drawString(64, 24, packet);
    factory_display.display();
  }
  delay(100);
  factory_display.clear();  
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
        if(deepsleep_state==false & gps_test_state==false & I2C_state==false)
        {
          showStatus(0);
        }
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
