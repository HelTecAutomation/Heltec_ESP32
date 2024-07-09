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
#include "LoRaWan_APP.h"
#include "driver/rtc_io.h"
#include <driver/gpio.h>

#define wakeuptime  10 * 1000 * (uint64_t)1000 //Sleep time

void intodeepsleep()
{
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  pinMode(18,OUTPUT);
  digitalWrite(18, LOW);
  Radio.Sleep();
  SPI.end();
  pinMode(14,ANALOG);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  rtc_gpio_hold_en(gpio_num_t(8));
  pinMode(12,ANALOG);
  pinMode(13,ANALOG);
  pinMode(9,ANALOG);
  pinMode(11,ANALOG);
  pinMode(10,ANALOG);
}

void setup()
{
    intodeepsleep();
    esp_sleep_enable_timer_wakeup(wakeuptime);
    delay(4000);
    esp_deep_sleep_start();
}
void loop()
{
}