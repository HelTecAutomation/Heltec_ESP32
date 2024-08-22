/* Heltec Automation LoRaWAN communication example
 *
 * Function:
 * 1. Drive GXHTV3 temperature and humidity sensor;
 * 2. Display current temperature and humidity on the TFT display;
 * 3. Upload temperature and humidity sensor via LoRaWAN protocol.
 *
 * Description:
 * 1. This example requires connecting the GXHTV3 sensor to the SH2.0-4P interface;
 * 2. To complete LoRaWAN communication, you need a LoRaWAN gateway and LoRaWAN server;
 * 3. Recommend LoRaWAN Gateway: https://heltec.org/project/ht-m7603/
 * 4. Recommend LoRaWAN Server: https://snapemu.com/
 *
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.org
 *
 * */

#include "LoRaWan_APP.h"
#include "HT_ST7789spi.h"
#include <Adafruit_GFX.h>  // Core graphics library
#include "img.h"
#include "Wire.h"
#include "GXHTC.h"
#define st7789_CS_Pin 39
#define st7789_REST_Pin 40
#define st7789_DC_Pin 47
#define st7789_SCLK_Pin 38
#define st7789_MOSI_Pin 48
#define st7789_LED_K_Pin 17
#define st7789_VTFT_CTRL_Pin 7
static HT_ST7789 *st7789 = NULL;  // lcd object pointer, it's a 240x135 lcd display, Adafruit dependcy
static SPIClass *gspi_lcd = NULL;

GXHTC gxhtc;
char buffer[256];

/* OTAA para*/
uint8_t devEui[] = { 0x22, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88 };

/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
uint32_t devAddr = (uint32_t)0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP*/
bool overTheAirActivation = 0;

/*ADR enable*/
bool loraWanAdr = true;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;
/*!
 * Number of trials to transmit the frame, if the LoRaMAC layer did not
 * receive an acknowledgment. The MAC performs a datarate adaptation,
 * according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
 * to the following table:
 *
 * Transmission nb | Data Rate
 * ----------------|-----------
 * 1 (first)       | DR
 * 2               | DR
 * 3               | max(DR-1,0)
 * 4               | max(DR-1,0)
 * 5               | max(DR-2,0)
 * 6               | max(DR-2,0)
 * 7               | max(DR-3,0)
 * 8               | max(DR-3,0)
 *
 * Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
 * the datarate, in case the LoRaMAC layer did not receive an acknowledgment
 */
uint8_t confirmedNbTrials = 4;

/* Prepares the payload of the frame */
static void prepareTxFrame(uint8_t port) {
  /*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
   *appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
   *if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
   *if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
   *for example, if use REGION_CN470,
   *the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
   */
  gxhtc.begin(2, 1);
  gxhtc.read_data();
  Serial.print("Temperature:");
  Serial.print(gxhtc.g_temperature);
  Serial.print("  Humidity:");
  Serial.println(gxhtc.g_humidity);
  st7789->drawRGBBitmap(0, 50, temp_72_0, 72, 72);
  sprintf(buffer, "%.2f", gxhtc.g_temperature);
  st7789->setTextSize(3);
    st7789->fillRect(70, 73, 100, 30, ST7789_BLACK);
  testdrawtext(70, 73, (char *)buffer, ST7789_RED);
  // testdrawtext(130, 100, "C", ST7789_WHITE);
  st7789->drawRGBBitmap(160, 50, Humidity_72_0, 72, 72);
  sprintf(buffer, "%.2f", gxhtc.g_humidity);
    st7789->fillRect(220, 73, 100, 30, ST7789_BLACK);
  testdrawtext(220, 73, (char *)buffer, ST7789_BLUE);
  // testdrawtext(280, 100, "%", ST7789_WHITE);
  appDataSize = 4;
  unsigned char *puc;
  appDataSize = 0;
  appData[appDataSize++] = 0x04;
  appData[appDataSize++] = 0x00;
  appData[appDataSize++] = 0x0A;
  appData[appDataSize++] = 0x02;
  puc = (unsigned char *)(&gxhtc.g_temperature);

  appData[appDataSize++] = puc[0];
  appData[appDataSize++] = puc[1];
  appData[appDataSize++] = puc[2];
  appData[appDataSize++] = puc[3];
  appData[appDataSize++] = 0x12;

  puc = (unsigned char *)(&gxhtc.g_humidity);
  appData[appDataSize++] = puc[0];
  appData[appDataSize++] = puc[1];
  appData[appDataSize++] = puc[2];
  appData[appDataSize++] = puc[3];

  Wire.end();

  appData[0] = 0x00;
  appData[1] = 0x01;
  appData[2] = 0x02;
  appData[3] = 0x03;
}
void testdrawtext(uint16_t x, uint16_t y, char *text, uint16_t color) {
  st7789->setCursor(x, y);
  st7789->setTextColor(color);
  st7789->setTextWrap(true);
  st7789->print(text);
}
// if true, next uplink will add MOTE_MAC_DEVICE_TIME_REQ

void setup() {
  Serial.begin(115200);
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  delay(20);
  gspi_lcd = new SPIClass(HSPI);
  st7789 = new HT_ST7789(240, 320, gspi_lcd, st7789_CS_Pin, st7789_DC_Pin, st7789_REST_Pin);
  gspi_lcd->begin(st7789_SCLK_Pin, -1, st7789_MOSI_Pin, st7789_CS_Pin);
  // set up slave select pins as outputs as the Arduino API
  pinMode(gspi_lcd->pinSS(), OUTPUT);
  st7789->init(170, 320);

  Serial.printf("Ready!\r\n");
  st7789->setRotation(1);
  st7789->fillScreen(ST7789_BLACK);
  testdrawtext(0, 0, "init >>> ", ST7789_WHITE);

  pinMode(17, OUTPUT);
  digitalWrite(17, HIGH);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);


  st7789->fillScreen(ST7789_BLACK);
  st7789->drawFastHLine(0, 15, 320, ST7789_WHITE);
  sprintf(buffer, "%.s", devEui);
//   st7789->setTextSize(1);
  // testdrawtext(0, 0, "deveui : 22233200000000", ST7789_WHITE);
//  st7789->drawRGBBitmap(270, 0, battery32_0, 32, 32);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
}

void loop() {
  switch (deviceState) {
    case DEVICE_STATE_INIT:
      {
#if (LORAWAN_DEVEUI_AUTO)
        LoRaWAN.generateDeveuiByChipID();
#endif
        LoRaWAN.init(loraWanClass, loraWanRegion);
        // both set join DR and DR when ADR off
        LoRaWAN.setDefaultDR(3);
        break;
      }
    case DEVICE_STATE_JOIN:
      {
        LoRaWAN.join();
        break;
      }
    case DEVICE_STATE_SEND:
      {
        prepareTxFrame(appPort);
        LoRaWAN.send();
        deviceState = DEVICE_STATE_CYCLE;
        break;
      }
    case DEVICE_STATE_CYCLE:
      {
        // Schedule next packet transmission
        txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;
      }
    case DEVICE_STATE_SLEEP:
      {
        LoRaWAN.sleep(loraWanClass);
        break;
      }
    default:
      {
        deviceState = DEVICE_STATE_INIT;
        break;
      }
  }
}