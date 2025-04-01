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

#include <Wire.h>
#include "MAX30102_PulseOximeter.h"
#include "LoRaWan_APP.h"
#define REPORTING_PERIOD_MS 1000
/* OTAA para*/
uint8_t devEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88 };
/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
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
uint8_t confirmedNbTrials = 4;
/* Pulse Oximeter object */
PulseOximeter pox;
uint32_t tsLastReport = 0;
/* Prepare LoRa payload with heart rate and SpO2 data */
static void prepareTxFrame(uint8_t port) {
pox.update();
float heartRate = pox.getHeartRate();
float spo2 = pox.getSpO2();
unsigned char *puc;
appDataSize = 0;
appData[appDataSize++] = 0x04;
appData[appDataSize++] = 0x00;
appData[appDataSize++] = 0x0A;
appData[appDataSize++] = 0x02;
puc = (unsigned char *)(&heartRate);
appData[appDataSize++] = puc[0];
appData[appDataSize++] = puc[1];
appData[appDataSize++] = puc[2];
appData[appDataSize++] = puc[3];
appData[appDataSize++] = 0x12;  // another identifier for SpO2
puc = (unsigned char *)(&spo2);
appData[appDataSize++] = puc[0];
appData[appDataSize++] = puc[1];
appData[appDataSize++] = puc[2];
appData[appDataSize++] = puc[3];
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
}
void setup() {
Serial.begin(115200);
Wire.begin(45, 46);
// Initialize LoRaWAN
Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
// MAX30102 Setup
Serial.print("Initializing MAX30102..");
delay(3000);
if (!pox.begin()) {
Serial.println("FAILED");
for(;;);
} else {
Serial.println("SUCCESS");
}
pox.setIRLedCurrent(MAX30102_LED_CURR_27_1MA);
pox.setOnBeatDetectedCallback(onBeatDetected);
// LoRaWAN initialization
LoRaWAN.init(loraWanClass, loraWanRegion);
LoRaWAN.setDefaultDR(3);  // Set datarate for LoRaWAN (can be adjusted)
}
void onBeatDetected() {
Serial.println("Beat detected!");
// Add additional debugging information
Serial.println("Heart rate sensor has detected a beat!");
}
void loop() {
pox.update(); // Update sensor data
if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
float heartRate = pox.getHeartRate();
float spo2 = pox.getSpO2();
if (heartRate > 0) {
Serial.print("Heart rate: ");
Serial.print(heartRate);
Serial.print(" bpm / ");
} else {
Serial.print("Heart rate: N/A / ");
}
if (spo2 > 0) {
Serial.print("SpO2: ");
Serial.print(spo2);
Serial.print(" %");
} else {
Serial.print("SpO2: N/A");
}
Serial.println();
tsLastReport = millis();
}
// LoRaWAN State Machine
switch (deviceState) {
case DEVICE_STATE_INIT:
{
// Handle joining of LoRaWAN network
LoRaWAN.join();
break;
}
case DEVICE_STATE_JOIN:
{
LoRaWAN.join();
break;
}
case DEVICE_STATE_SEND:
{
// Prepare and send data over LoRaWAN
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
