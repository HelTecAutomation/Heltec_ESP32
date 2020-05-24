/* Heltec Automation ESP32 external PSRAM read/write example
 *
 * Must have external PSRAM connected.
 * 
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
 */


#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  
  Serial.printf("Total heap: %d\r\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d\r\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d\r\n", ESP.getPsramSize());
  byte* psdRamBuffer = (byte*)ps_malloc(4000000);
  Serial.printf("Free PSRAM: %d\r\n", ESP.getFreePsram());
}

void loop() {}
