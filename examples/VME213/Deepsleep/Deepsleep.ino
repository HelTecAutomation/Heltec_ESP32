/* Heltec Automation adc  example
 *
 * Function:
 * 1.ESP32 deep sleep achieves low power consumption
 * 
 * * Description:
 * 1.Pin states can be set to achieve stable deep sleep.
 * 
 * Library url: https://github.com/HelTecAutomation/Heltec_ESP32
 * Support: support@heltec.cn
 *
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * https://www.heltec.org
 *
 * */

#include "Arduino.h"

#define wakeuptime  600 * 1000 * (uint64_t)1000 //Sleep time
void setup()
{
    // digitalWrite(RADIO_NSS, HIGH);
    // gpio_hold_en((gpio_num_t)RADIO_NSS);
    // pinMode(RADIO_RESET, ANALOG);
    // pinMode(RADIO_BUSY, ANALOG);
    // pinMode(LORA_CLK, ANALOG);
    // pinMode(LORA_MISO, ANALOG);
    // pinMode(LORA_MOSI, ANALOG);
    esp_sleep_enable_timer_wakeup(wakeuptime);
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_deep_sleep_start();
}
void loop()
{
}