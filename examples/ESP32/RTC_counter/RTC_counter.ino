#include "Arduino.h"
#include "soc/rtc.h"

/*rtc_time_get() return the RTC counters
* the board using internal 150KHz RTC, but it's not accurate.
*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  uint64_t rtc_counter1 = rtc_time_get();
  delay(1000);
  uint64_t rtc_counter2 = rtc_time_get();
  
  Serial.println((uint32_t)(rtc_counter2-rtc_counter1));
}