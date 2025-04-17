#include <DHTesp.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"

// DHT11 Configuration
#define DHT_PIN 1  // GPIO1 connected to DHT11 (avoid using GPIO1)
DHTesp dht;

// LoRa Configuration
#define RF_FREQUENCY                915000000   // Hz (adjust according to your region)
#define TX_OUTPUT_POWER             14          // dBm (increase power to extend transmission range)
#define LORA_BANDWIDTH              0           // 125 kHz
#define LORA_SPREADING_FACTOR       7           // SF7
#define LORA_CODINGRATE             1           // 4/5
#define LORA_PREAMBLE_LENGTH        8           // Preamble length
#define LORA_FIX_LENGTH_PAYLOAD_ON  false
#define LORA_IQ_INVERSION_ON        false

#define BUFFER_SIZE                 50          // Data buffer size
char txpacket[BUFFER_SIZE];                    // Transmission buffer
bool lora_idle = true;                         // LoRa idle state flag

// LoRa event callbacks
static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT11
  dht.setup(DHT_PIN, DHTesp::DHT11);
  Serial.println("DHT11 Initialized");

  // Initialize LoRa
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE); // Adjust parameters according to your board
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
}

void loop() {
  if (lora_idle) {
    delay(2000); // DHT11 requires at least 1-second sampling interval

    // Read temperature and humidity data
    TempAndHumidity data = dht.getTempAndHumidity();
    if (dht.getStatus() != DHTesp::ERROR_NONE) {
      Serial.println("DHT11 read failed: " + String(dht.getStatusString()));
      return;
    }

    // Format data as JSON string (example: {"t":25.5,"h":50.0})
    snprintf(txpacket, BUFFER_SIZE, 
             "{\"t\":%.1f,\"h\":%.1f}", 
             data.temperature, 
             data.humidity);

    Serial.printf("Sending data: %s\n", txpacket);

    // Transmit data via LoRa
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    lora_idle = false;
  }
  Radio.IrqProcess(); // Process LoRa events
}

// LoRa transmission complete callback
void OnTxDone(void) {
  Serial.println("LoRa transmission successful");
  lora_idle = true;
}

// LoRa transmission timeout callback
void OnTxTimeout(void) {
  Radio.Sleep();
  Serial.println("LoRa transmission timeout");
  lora_idle = true;
}