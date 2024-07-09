/* Heltec Automation LoRaWAN communication example
 *
 * Function:
 * 1. Upload node data to the server using the standard LoRaWAN protocol.
 * 2. Accept and display messages on the screen
 * 3. Calibrate the time from the network and then use an internal crystal oscillator to determine the time
 * 4. Read sensor data and upload it to the server. Receive server messages and display them on the screen.
 * 
 * Description:
 * 1. Communicate using LoRaWAN protocol.
 * 
 * Library url: https://github.com/HelTecAutomation/Heltec_ESP32
 * Support: support@heltec.cn
 *
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * https://www.heltec.org
 * */

#include "LoRaWan_APP.h"
#include "Wire.h"
#include "GXHTC.h"
#include "HT_lCMEN2R13EFC1.h"
#include "img.h"
#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>

GXHTC gxhtc;
HT_ICMEN2R13EFC1 display(3, 2, 5, 1, 4, 6, -1, 6000000); // rst,dc,cs,busy,sck,mosi,miso,frequency

char buffer[256];
const char *ntpServer = "ntp.aliyun.com";
const long gmtOffset_sec = 28800;  //Time Offset
const int daylightOffset_sec = 0;
const char *ssid = "Your Wi-Fi SSID";
const char *password = "Your Wi-Fi password";
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
DeviceClass_t loraWanClass = CLASS_C;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000*4*2;

/*OTAA or ABP*/
bool overTheAirActivation = true;

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
  display.clear();
  display.drawLine(0, 15, 250, 15);
  GetNetTime();
  gxhtc.begin(39, 38);
  gxhtc.read_data();
  Serial.print("Temperature:");
  Serial.print(gxhtc.g_temperature);
  Serial.print("Humidity:");
  Serial.println(gxhtc.g_humidity);
  display.setFont(ArialMT_Plain_24);
  sprintf(buffer, "%.2f", gxhtc.g_temperature);
  display.drawXbm(18, 55, 10, 17, temp);
  display.drawString(30, 50, buffer);
  sprintf(buffer, "%.2f", gxhtc.g_humidity);
  display.drawXbm(128, 55, 11, 16, hum);
  display.drawString(142, 50, buffer);
  display.update(COLOR_BUFFER);
  display.display();
  delay(3000);
  delay(100);
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
}

//if true, next uplink will add MOTE_MAC_DEVICE_TIME_REQ
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  display.setFont(ArialMT_Plain_10);
  char buffer[256];
  display.setFont(ArialMT_Plain_10);
  Serial.println(timeinfo.tm_hour);
  sprintf(buffer, "%d", timeinfo.tm_hour);
  display.drawString(10, 0, buffer);
  display.drawString(25, 0, ": ");
  sprintf(buffer, "%d", timeinfo.tm_min);
  Serial.println(timeinfo.tm_min);
  display.drawString(30, 0, buffer);
  Serial.println(timeinfo.tm_mon);
  display.setFont(ArialMT_Plain_16);
  sprintf(buffer, "%d", timeinfo.tm_year + 1900);  //Starting from 1900, year counting
  display.drawString(160, 100, buffer);
  display.drawString(195, 100, "/");
  sprintf(buffer, "%d", timeinfo.tm_mon + 1);
  display.drawString(200, 100, buffer);
  display.drawString(210, 100, "/");
  sprintf(buffer, "%d", timeinfo.tm_mday);
  Serial.println(timeinfo.tm_mday);
  display.drawString(215, 100, buffer);
  Serial.println(timeinfo.tm_mday);
}

void GetNetTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void downLinkDataHandle(McpsIndication_t *mcpsIndication) {
  char data[256];
  Serial.printf("+REV DATA:%s,RXSIZE %d,PORT %d\r\n", mcpsIndication->RxSlot ? "RXWIN2" : "RXWIN1", mcpsIndication->BufferSize, mcpsIndication->Port);
  Serial.print("+REV DATA:");
  for (uint8_t i = 0; i < mcpsIndication->BufferSize; i++) {
  Serial.printf("%02X", mcpsIndication->Buffer[i]);
  }
  sprintf(data,"%02x",mcpsIndication->Buffer);
  display.drawString(10, 100, "data :");
  display.drawString(60, 100,data);
  display.update(COLOR_BUFFER);
  display.display();
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.begin(115200);
  pinMode(18, OUTPUT);
  digitalWrite(18, HIGH);
  gxhtc.begin(39, 38);
  display.init();
  display.clear();
  display.update(COLOR_BUFFER);
  display.drawString(0, 0, "init >>> ");
  Serial.print("Connecting to ");
  display.drawString(0, 20, "Connecting to ... ");
  Serial.println(ssid);
  display.drawString(100, 20, ssid);
  WiFi.begin(ssid, password);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.drawString(i, 40, ".");
    i = i + 10;
  }
  Serial.println("");
  Serial.println("WiFi connected");
  display.drawString(0, 60, "WiFi connected");
  Serial.println("IP address: ");
  display.drawString(0, 90, "IP address: ");
  display.drawString(60, 90, WiFi.localIP().toString().c_str());
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.println("WiFi Conected!");
  display.update(COLOR_BUFFER);
  display.display();
  delay(5000);
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
        //both set join DR and DR when ADR off
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