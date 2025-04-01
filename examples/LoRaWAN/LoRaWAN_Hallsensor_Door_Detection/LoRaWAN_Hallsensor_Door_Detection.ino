/* Heltec Automation LoRaWAN communication example
 *
 * Function:
 * 1. Upload node data to the server using the standard LoRaWAN protocol.
 *  
 * Description:
 * 1. Communicate using LoRaWAN protocol.
 * 
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.org
 *
 * */

#include "LoRaWan_APP.h"
int sensorData = 0;
uint8_t confirmedNbTrials = 8;
const int hallSensorPin = 21;                    // Connect the GPIO pin of the Hall sensor
volatile int counter = 0;                        // Counter, used to record the number of changes in high and low levels
volatile unsigned long lastInterruptTime = 0;    // Record the last trigger time
const unsigned long debounceTime = 50;           // Anti shake delay time (milliseconds)
const unsigned long measurementInterval = 30000; // Measurement time interval (30 seconds)

unsigned long startTime = 0;                     // Timing start time
/* OTAA para*/
uint8_t devEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE8 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88 };

/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

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

void IRAM_ATTR handleInterrupt() {
  unsigned long currentTime = millis();
  
  
  if (currentTime - lastInterruptTime > debounceTime) {
    counter++;
    lastInterruptTime = currentTime; 
 }
}

/* Prepares the payload of the frame */

static void prepareTxFrame(uint8_t port) {
    if (millis() - startTime >= measurementInterval) {
        Serial.printf("Switching frequency: %d\n", counter);
        
        // Encapsulate Hall sensor data counter
        appDataSize = 0; 
        unsigned char*puc;
        appData[appDataSize++] = 0x00; // parent ID
        appData[appDataSize++] = 0x00; // parent ID
        appData[appDataSize++] = 0x05; // sensor length
        appData[appDataSize++] = 0x00; 

        puc = (unsigned char *)(&counter); 
        appData[appDataSize++] = puc[3];
        appData[appDataSize++] = puc[2];
        appData[appDataSize++] = puc[1];
        appData[appDataSize++] = puc[0];

        Serial.print("Sending data: ");
        for (int i = 0; i < appDataSize; i++) {
            Serial.print(appData[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        // Ensure that the total size does not exceed the maximum limit
        if (appDataSize > LORAWAN_APP_DATA_MAX_SIZE) {
            appDataSize = LORAWAN_APP_DATA_MAX_SIZE;
        }
        counter = 0; 
        startTime = millis(); 
    }
}
 
void setup() {
  Serial.begin(115200);
   while (!Serial) {
  delay(10); 
}
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  pinMode(hallSensorPin, INPUT);  
  attachInterrupt(digitalPinToInterrupt(hallSensorPin), handleInterrupt, RISING);  
  startTime = millis(); 
}

void loop()
{
  switch( deviceState )
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