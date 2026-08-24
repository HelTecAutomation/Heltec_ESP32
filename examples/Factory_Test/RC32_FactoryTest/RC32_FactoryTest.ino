/*
 * HelTec Automation(TM) RC32 factory test code.
 *
 * Serial-only factory flow:
 * - GPIO sequential sweep
 * - WiFi connect and scan
 * - battery ADC check
 * - LoRa DUT protocol check
 * - button-triggered low power entry
 */

#include "Arduino.h"
#include "WiFi.h"
#include "LoRaWan_APP.h"
#include <SPI.h>
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_mac.h"

#if !defined(HELTEC_RC32)
#error "RC32_FactoryTest is intended for Heltec RC32."
#endif

#ifndef USER_BUTTON
#define USER_BUTTON 0
#endif

#define FACTORY_FW_VERSION "RC32FT1"
#define SERIAL_BAUDRATE 115200

#define TEST_WIFI_SSID "TP-LINK_74864531"
#define TEST_WIFI_PWD "heltec_test"
#define WIFI_CONNECT_TRIES 10
#define WIFI_CONNECT_STEP_MS 500UL

#define RF_FREQUENCY_1 868100000
#define RF_FREQUENCY_2 870900000
#define TX_OUTPUT_POWER 20
#define LORA_BANDWIDTH 0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define LORA_BUFFER_SIZE 30
#define LORA_DUT_TIMEOUT_MS 60000UL
#define LORA_RSSI_DELTA_MAX 15

#define IO_STEP_DELAY_MS 500UL
#define ADC_SAMPLE_COUNT 50
#define ADC_PASS_MIN_MV 3700
#define ADC_PASS_MAX_MV 3900
#define BATTERY_MULTIPLIER 4.9f

typedef enum {
  LOWPOWER,
  STATE_RX,
  STATE_TX
} States_t;

struct TestIoPin {
  const char *name;
  int16_t pin;
};

static const TestIoPin testIoPins[] = {
    {"45", 45},
    {"4", 4},
    {"40", 40},
    {"39", 39},
    {"38", 38},
    {"5", 5},
    {"2", 2},
    {"48", 48},
    {"47", 47},
    {"21", 21},
    {"16", 16},
    {"41", 41},
    {"TX", TX},
    {"RX", RX},
    {"18", 18},
    {"46", 46},
    {"6", 6},
    {"17", 17},
    {"42", 42},
};

static RadioEvents_t RadioEvents;
static States_t state = LOWPOWER;
static uint8_t rxpacket[LORA_BUFFER_SIZE];

static uint64_t chipId = 0;
static char chipIdText[17] = {0};
static int16_t maxTxRssi = -255;
static int16_t maxRxRssi = -255;
static uint8_t rxCount = 0;
static bool loraTimeout = false;
static bool loraDone = false;
static bool loraPass = false;
static bool loraStarted = false;
static bool loraReported = false;
static uint32_t loraStartMs = 0;

static bool ioPass = true;
static bool wifiConnectPass = false;
static bool wifiScanPass = false;
static bool adcPass = false;
static uint16_t adcMilliVolts = 0;
static uint16_t batteryMilliVolts = 0;
static int wifiScanCount = 0;
static int wifiScanMaxRssi = -255;

static void makeChipIdText()
{
  uint8_t mac[6] = {0};
  esp_base_mac_addr_get(mac);
  chipId = ((uint64_t)mac[0] << 40) |
           ((uint64_t)mac[1] << 32) |
           ((uint64_t)mac[2] << 24) |
           ((uint64_t)mac[3] << 16) |
           ((uint64_t)mac[4] << 8) |
           ((uint64_t)mac[5]);
  snprintf(chipIdText, sizeof(chipIdText), "%04X%08X",
           (uint16_t)(chipId >> 32), (uint32_t)chipId);
}

static void printItem(const char *id, bool pass, const String &detail)
{
  Serial.print("FT_ITEM,");
  Serial.print(id);
  Serial.print(",");
  Serial.print(pass ? "PASS" : "FAIL");
  if (detail.length() > 0) {
    Serial.print(",");
    Serial.print(detail);
  }
  Serial.println();
}

static bool userPressed()
{
  if (digitalRead(USER_BUTTON) != LOW) {
    return false;
  }
  delay(30);
  return digitalRead(USER_BUTTON) == LOW;
}

static void waitUserReleased()
{
  pinMode(USER_BUTTON, INPUT_PULLUP);
  while (digitalRead(USER_BUTTON) == LOW) {
    delay(5);
  }
  delay(40);
}

static void setTestIoPinsAnalog()
{
  for (size_t i = 0; i < sizeof(testIoPins) / sizeof(testIoPins[0]); i++) {
    if (testIoPins[i].pin >= 0) {
      pinMode(testIoPins[i].pin, ANALOG);
    }
  }
}

static void setRc32PeripheralPinsForSleep()
{
  setTestIoPinsAnalog();

  pinMode(USER_BUTTON, ANALOG);
  pinMode(ADC_BATTERY_PIN, ANALOG);

  pinMode(RADIO_DIO_1, ANALOG);
  pinMode(RADIO_NSS, ANALOG);
  pinMode(RADIO_RESET, ANALOG);
  pinMode(RADIO_BUSY, ANALOG);
  pinMode(LORA_CLK, ANALOG);
  pinMode(LORA_MISO, ANALOG);
  pinMode(LORA_MOSI, ANALOG);
}

static void runIoSweep()
{
  Serial.println("FT_ITEM,IO_SWEEP,START,MODE=SEQUENTIAL_500MS");
  waitUserReleased();

  for (size_t i = 0; i < sizeof(testIoPins) / sizeof(testIoPins[0]); i++) {
    if (testIoPins[i].pin < 0) {
      continue;
    }
    pinMode(testIoPins[i].pin, OUTPUT);
    digitalWrite(testIoPins[i].pin, LOW);
  }

  for (size_t i = 0; i < sizeof(testIoPins) / sizeof(testIoPins[0]); i++) {
    if (testIoPins[i].pin < 0) {
      Serial.println("FT_IO,NC,SKIP");
      delay(IO_STEP_DELAY_MS);
      continue;
    }
    digitalWrite(testIoPins[i].pin, HIGH);
    Serial.print("FT_IO,");
    Serial.print(testIoPins[i].name);
    Serial.print(",HIGH,PIN=");
    Serial.println(testIoPins[i].pin);
    delay(IO_STEP_DELAY_MS);
  }

  for (size_t i = 0; i < sizeof(testIoPins) / sizeof(testIoPins[0]); i++) {
    if (testIoPins[i].pin < 0) {
      Serial.println("FT_IO,NC,SKIP");
      delay(IO_STEP_DELAY_MS);
      continue;
    }
    digitalWrite(testIoPins[i].pin, LOW);
    Serial.print("FT_IO,");
    Serial.print(testIoPins[i].name);
    Serial.print(",LOW,PIN=");
    Serial.println(testIoPins[i].pin);
    delay(IO_STEP_DELAY_MS);
  }

  pinMode(USER_BUTTON, INPUT_PULLUP);
  printItem("IO_SWEEP", ioPass, "ERR=NONE");
}

static bool runWifiConnect()
{
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(TEST_WIFI_SSID, TEST_WIFI_PWD);

  for (uint8_t count = 0; count < WIFI_CONNECT_TRIES; count++) {
    if (WiFi.status() == WL_CONNECTED) {
      String detail = "SSID=" + String(TEST_WIFI_SSID) +
                      ",IP=" + WiFi.localIP().toString() +
                      ",RSSI=" + String(WiFi.RSSI()) +
                      ",ERR=NONE";
      printItem("WIFI_CONNECT", true, detail);
      return true;
    }
    delay(WIFI_CONNECT_STEP_MS);
  }

  printItem("WIFI_CONNECT", false,
            "SSID=" + String(TEST_WIFI_SSID) + ",ERR=CONNECT_TIMEOUT");
  return false;
}

static bool runWifiScan()
{
  WiFi.mode(WIFI_STA);
  wifiScanCount = WiFi.scanNetworks();
  wifiScanMaxRssi = -255;

  for (int i = 0; i < wifiScanCount; i++) {
    if (wifiScanMaxRssi < WiFi.RSSI(i)) {
      wifiScanMaxRssi = WiFi.RSSI(i);
    }
  }

  bool pass = wifiScanCount > 0;
  String detail = "CNT=" + String(wifiScanCount) +
                  ",MAX_RSSI=" + String(wifiScanMaxRssi) +
                  ",ERR=" + String(pass ? "NONE" : "NO_AP");
  printItem("WIFI_SCAN", pass, detail);
  WiFi.scanDelete();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  return pass;
}

static bool readAdc()
{
  uint32_t adcMilliVoltSum = 0;
  analogReadResolution(12);

  pinMode(ADC_BATTERY_CTRL_PIN, OUTPUT);
  digitalWrite(ADC_BATTERY_CTRL_PIN, HIGH);
  delay(10);

  for (uint8_t i = 0; i < ADC_SAMPLE_COUNT; i++) {
    adcMilliVoltSum += analogReadMilliVolts(ADC_BATTERY_PIN);
    delay(2);
  }

  digitalWrite(ADC_BATTERY_CTRL_PIN, LOW);
  pinMode(ADC_BATTERY_PIN, ANALOG);

  adcMilliVolts = (uint16_t)(adcMilliVoltSum / ADC_SAMPLE_COUNT);
  batteryMilliVolts = (uint16_t)(adcMilliVolts * BATTERY_MULTIPLIER);
  bool pass = batteryMilliVolts >= ADC_PASS_MIN_MV &&
              batteryMilliVolts <= ADC_PASS_MAX_MV;
  String detail = "ADC_MV=" + String(adcMilliVolts) +
                  ",BAT_MV=" + String(batteryMilliVolts) +
                  ",ERR=" + String(pass ? "NONE" : "MV_RANGE");
  printItem("ADC", pass, detail);
  return pass;
}

void OnTxDone(void)
{
  Serial.println("TX done......");
  state = STATE_RX;
}

void OnTxTimeout(void)
{
  Radio.Sleep();
  Serial.println("TX Timeout......");
  state = STATE_TX;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  (void)snr;
  Radio.Sleep();
  Serial.print("RX done...... Rx size: ");
  Serial.print(size);
  Serial.print(" rssi: ");
  Serial.println(rssi);

  if (size == 10) {
    uint64_t rxChipId = 0;
    int16_t txRssi = 0;
    memcpy(&rxChipId, payload, sizeof(rxChipId));
    memcpy(&txRssi, payload + 8, sizeof(txRssi));

    if (rxChipId == chipId) {
      memcpy(rxpacket, payload, size);
      if (maxTxRssi < txRssi) {
        maxTxRssi = txRssi;
      }
      if (maxRxRssi < rssi) {
        maxRxRssi = rssi;
      }
      rxCount++;
      Serial.print("FT_LORA_RX,CHIPID=");
      Serial.print(chipIdText);
      Serial.print(",TX_RSSI=");
      Serial.print(txRssi);
      Serial.print(",RX_RSSI=");
      Serial.print(rssi);
      Serial.print(",RX_CNT=");
      Serial.println(rxCount);
      if (rxCount >= 3) {
        loraPass = abs(abs(maxTxRssi) - abs(maxRxRssi)) <= LORA_RSSI_DELTA_MAX;
        loraDone = true;
        state = LOWPOWER;
        return;
      }
    }
  }

  state = STATE_TX;
}

void OnRxTimeout(void)
{
  Radio.Sleep();
  Serial.println("RX Timeout......");
  state = STATE_TX;
}

void OnRxError(void)
{
  Radio.Sleep();
  Serial.println("RX Error......");
  state = STATE_TX;
}

static void loraInit()
{
  if (loraStarted) {
    return;
  }
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.RxError = OnRxError;
  RadioEvents.RxTimeout = OnRxTimeout;
  Radio.Init(&RadioEvents);
  srand1(Radio.Random());
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON, true, 0,
                    0, LORA_IQ_INVERSION_ON, 3000);
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0, true, 0,
                    0, LORA_IQ_INVERSION_ON, true);
  state = STATE_TX;
  loraStarted = true;
  loraStartMs = millis();
  Serial.println("FT_ITEM,LORA,START,ROLE=DUT");
}

static void loraLoop()
{
  if (loraDone) {
    return;
  }

  if ((millis() - loraStartMs) > LORA_DUT_TIMEOUT_MS && rxCount < 3) {
    if (!loraTimeout) {
      loraTimeout = true;
      loraDone = true;
      loraPass = false;
      state = LOWPOWER;
    }
    return;
  }

  switch (state) {
  case STATE_TX:
    delay(1000 + randr(0, 100));
    Radio.SetChannel(RF_FREQUENCY_1);
    Serial.println("lora DUT: into TX mode");
    Radio.Send((uint8_t *)&chipId, 8);
    state = LOWPOWER;
    break;
  case STATE_RX:
    Radio.SetChannel(RF_FREQUENCY_2);
    Serial.println("lora DUT: into RX mode");
    Radio.Rx(5000);
    state = LOWPOWER;
    break;
  case LOWPOWER:
    Mcu.timerhandler();
    Radio.IrqProcess();
    break;
  default:
    break;
  }
}

static void reportLoraResult()
{
  String err = "NONE";
  if (!loraPass) {
    err = loraTimeout ? "LORA_TIMEOUT" : "RSSI_DELTA";
  }
  String detail = "TX_RSSI=" + String(maxTxRssi) +
                  ",RX_RSSI=" + String(maxRxRssi) +
                  ",RX_CNT=" + String(rxCount) +
                  ",ERR=" + err;
  printItem("LORA", loraPass, detail);
}

static void enterDeepSleep()
{
  Serial.println("FT_ITEM,LOWPOWER,READY");

  Radio.Sleep();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  SPI.end();

  pinMode(ADC_BATTERY_CTRL_PIN, OUTPUT);
  digitalWrite(ADC_BATTERY_CTRL_PIN, LOW);

  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);

  Serial.println("FT_RESULT,LOWPOWER,ENTER");
  Serial.flush();
  setRc32PeripheralPinsForSleep();
  esp_deep_sleep_start();
}

static void printFinalResult()
{
  Serial.print("FT_RESULT,");
  Serial.print((ioPass && wifiConnectPass && wifiScanPass && adcPass && loraPass)
                   ? "PASS"
                   : "FAIL");
  Serial.print(",CHIPID=");
  Serial.print(chipIdText);
  Serial.print(",WIFI_CONNECTED=");
  Serial.print(wifiConnectPass ? 1 : 0);
  Serial.print(",WIFI_SCAN_CNT=");
  Serial.print(wifiScanCount);
  Serial.print(",WIFI_MAX_RSSI=");
  Serial.print(wifiScanMaxRssi);
  Serial.print(",ADC_MV=");
  Serial.print(adcMilliVolts);
  Serial.print(",BAT_MV=");
  Serial.print(batteryMilliVolts);
  Serial.print(",LORA_RX_CNT=");
  Serial.println(rxCount);
  Serial.println("FT_ITEM,LOWPOWER,READY");
}

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
  makeChipIdText();
  pinMode(USER_BUTTON, INPUT_PULLUP);

  Serial.print("FT_START,BOARD=Heltec RC32,FW=");
  Serial.print(FACTORY_FW_VERSION);
  Serial.print(",ROLE=DUT,CHIPID=");
  Serial.println(chipIdText);

  runIoSweep();
  wifiConnectPass = runWifiConnect();
  wifiScanPass = runWifiScan();
  adcPass = readAdc();
  waitUserReleased();
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  loraInit();
}

void loop()
{
  if (loraReported) {
    if (userPressed()) {
      enterDeepSleep();
    }
    delay(50);
    return;
  }

  loraLoop();

  if (loraDone) {
    reportLoraResult();
    printFinalResult();
    loraReported = true;
  }
}
