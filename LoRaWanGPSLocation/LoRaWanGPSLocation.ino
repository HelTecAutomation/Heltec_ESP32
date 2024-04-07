/* Heltec Automation LoRaWAN communication example
 *
 * Function:
 * 1. Read the longitude and latitude of GPS and upload it to the LoRaWAN server.
 * 2. Display the longitude and latitude of GPS on the screen.
 *
 * Description:
 * 1. Communicate using LoRaWAN protocol.
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
#include "HT_TinyGPS++.h"

TinyGPSPlus GPS;
HT_st7735 st7735;

#define VGNSS_CTRL 3
/* OTAA para*/
uint8_t devEui[] = {0x22, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x23};
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
uint32_t license[4] = {0x21701168, 0xAAAEEC0A, 0xFDA9F9D2, 0x0110E29E};
/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP*/
bool overTheAirActivation = true;

/*ADR enable*/
bool loraWanAdr = true;
/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;
/*!
  Number of trials to transmit the frame, if the LoRaMAC layer did not
  receive an acknowledgment. The MAC performs a datarate adaptation,
  according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
  to the following table:

  Transmission nb | Data Rate
  ----------------|-----------
  1 (first)       | DR
  2               | DR
  3               | max(DR-1,0)
  4               | max(DR-1,0)
  5               | max(DR-2,0)
  6               | max(DR-2,0)
  7               | max(DR-3,0)
  8               | max(DR-3,0)

  Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
  the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;
void GPS_test(void)
{
  Serial.println("GPS_test");
  st7735.st7735_fill_screen(ST7735_BLACK);
  delay(100);
  st7735.st7735_write_str(0, 0, (String)"GPS_test");

    if (Serial1.available() > 0)
    {
      if (Serial1.peek() != '\n')
      {
        GPS.encode(Serial1.read());
      }
      else
      {
        Serial1.read();
     }
    }
  }

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
  /*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commiSerial1ioning.h".
    appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
    if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
    if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
    for example, if use REGION_CN470,
    the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
  */

  // pinMode(GPIO0, OUTPUT);
  pinMode(Vext, OUTPUT);
  // digitalWrite(GPIO0, HIGH);
  digitalWrite(Vext, HIGH);

  float lat, lon, alt, course, speed, hdop, sats;

  Serial.println("Waiting for GPS FIX ...");
 
  while (!GPS.location.isValid()) 
  {
    //smartDelay(1000);
    uint32_t start = millis();
    do 
    {
      if (Serial1.available()) 
      {
        GPS.encode(Serial1.read());
      }
     } while(GPS.charsProcessed() < 10); //((millis() + start) <5000); 

    if ((millis() - start) > 10000)// && GPS.charsProcessed() < 10) 
    {
     Serial.println("No GPS data received: check wiring");
     break;
    }
  }

  lat = GPS.location.lat();
  lon = GPS.location.lng();
  digitalWrite(Vext, LOW);
  // digitalWrite(GPIO0, LOW);

  // pinMode(GPIO0, ANALOG);
  // uint16_t batteryVoltage = getBatteryVoltage();

  unsigned char *puc;

  appDataSize = 0;
  puc = (unsigned char *)(&lat);
  appData[appDataSize++] = puc[0];
  appData[appDataSize++] = puc[1];
  appData[appDataSize++] = puc[2];
  appData[appDataSize++] = puc[3];
  puc = (unsigned char *)(&lon);
  appData[appDataSize++] = puc[0];
  appData[appDataSize++] = puc[1];
  appData[appDataSize++] = puc[2];
  appData[appDataSize++] = puc[3];

  Serial.print(", LAT: ");
  Serial.print(GPS.location.lat());
  Serial.print(", LON: ");
  Serial.print(GPS.location.lng());
}




void setup()
{
 
  Serial1.begin(115200, SERIAL_8N1, 33, 34);
   Serial.begin(115200);
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
  Mcu.setlicense(license);
 st7735.st7735_init();
 st7735.st7735_fill_screen(ST7735_BLACK);
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);

}
void loop()
{
  switch ( deviceState )
  {
    case DEVICE_STATE_INIT:
      {
#if(LORAWAN_DEVEUI_AUTO)
        LoRaWAN.generateDeveuiByChipID();
#endif
      LoRaWAN.init(loraWanClass,loraWanRegion);
      //both set join DR and DR when ADR off 
      LoRaWAN.setDefaultDR(3);
      break;
    }
    case DEVICE_STATE_JOIN:
      {
        LoRaWAN.join();
		st7735.st7735_write_str(0, 0, "join>>>");

        break;
      }
    case DEVICE_STATE_SEND:
      {
        prepareTxFrame( appPort );
        		st7735.st7735_write_str(0, 0, "send>>>");

        LoRaWAN.send();       
         		st7735.st7735_write_str(0, 0, "send>>>    ok");


        deviceState = DEVICE_STATE_CYCLE;
        break;
      }
    case DEVICE_STATE_CYCLE:
      {
        txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
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
