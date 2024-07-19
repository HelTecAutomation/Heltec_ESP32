/* Heltec Automation adc  example
 *
 * Function:
 * 1.Read the battery voltage and display it on the screen
 * 
 * * Description:
 * 1.ADC reading uses a 100k/390k voltage division ratio, and the maximum voltage after voltage division is 3.3v
 * 
 * Library url: https://github.com/HelTecAutomation/Heltec_ESP32
 * Support: support@heltec.cn
 *
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * https://www.heltec.org
 *
 * */
#include "HT_lCMEN2R13EFC1.h"
#include "images.h"

HT_ICMEN2R13EFC1 display(3, 2, 5, 1, 4, 6, -1, 6000000); // rst,dc,cs,busy,sck,mosi,miso,frequency
 
/* screen rotation
 * ANGLE_0_DEGREE
 * ANGLE_90_DEGREE
 * ANGLE_180_DEGREE
 * ANGLE_270_DEGREE
 */
#define DIRECTION ANGLE_0_DEGREE
void setup()
{
    Serial.begin(115200);
    
    VextON();
    delay(100);
    // Initialising the UI will init the display too.
    display.init();
    display.screenRotate(DIRECTION);
    display.setFont(ArialMT_Plain_10);
}
void battery()
{
    analogReadResolution(12);
    float battery_levl = analogReadMilliVolts(7)* 0.001 * 5.09; //battary/4096*3.3* coefficient
    float battery_one = 0.528;
    uint8_t bl= battery_levl*100;
    uint8_t bo= battery_one*100;

    if (bl < bo)
    {
        display.drawString(230, 0, "N/A");
        display.drawXbm(215, 0, battery_w, battery_h, battery0);
    }
    else if (bl < 2 * bo && bl > bo)
    {
          Serial.printf("ADC analog value = %.2f\n", battery_levl );

        display.drawXbm(230, 0, battery_w, battery_h, battery1);
    }
    else if (bl < 3 * bo && bl > 2 * bo)
    {
          Serial.printf("ADC analog value = %.2f\n", battery_levl );

        display.drawXbm(230, 0, battery_w, battery_h, battery2);
    }
    else if (bl < 4 * bo && bl > 3 * bo)
    {
          Serial.printf("ADC analog value = %.2f\n", battery_levl );

        display.drawXbm(230, 0, battery_w, battery_h, battery3);
    }
    else if (bl < 5 * bo && bl > 4 * bo)
    {
          Serial.printf("ADC analog value = %.2f\n", battery_levl );

        display.drawXbm(230, 0, battery_w, battery_h, battery4);
    }
    else if (bl < 6 * bo && bl > 5 * bo)
    {
          Serial.printf("ADC analog value = %.2f\n", battery_levl );

        display.drawXbm(230, 0, battery_w, battery_h, battery5);
    }
    else if (bl < 7 * bo && bl > 6 * bo)
    {
          Serial.printf("ADC analog value = %.2f\n", battery_levl );

        display.drawXbm(230, 0, battery_w, battery_h, battery6);
    }
    else if (bl < 7 * bo && bl > 6 * bo)
    {
          Serial.printf("ADC analog value = %.2f\n", battery_levl );

        display.drawXbm(230, 0, battery_w, battery_h, batteryfull);
    }
}

void VextON(void)
{
    pinMode(18, OUTPUT);
    digitalWrite(18, HIGH);
}

void VextOFF(void) // Vext default OFF
{
    pinMode(18, OUTPUT);
    digitalWrite(18, LOW);
}

void loop()
{
    display.clear();
    display.update(COLOR_BUFFER);
    display.clear();
    display.drawLine(0, 15, 250, 15);
    battery();
    display.update(COLOR_BUFFER);
    display.display();
    delay(10000);
}
