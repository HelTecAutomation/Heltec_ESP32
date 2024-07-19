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
#include "HT_DEPG0290BxS800FxX_BW.h"
#include "images.h"
// Initialize the display
DEPG0290BxS800FxX_BW   display(5, 4, 3,6, 2,1,-1, 6000000);//rst,dc,cs,busy,sck,mosi,miso,frequency
 
/* screen rotation
 * ANGLE_0_DEGREE
 * ANGLE_90_DEGREE
 * ANGLE_180_DEGREE
 * ANGLE_270_DEGREE
 */
#define DIRECTION ANGLE_0_DEGREE
#define Resolution 0.000244140625 
#define battary_in 3.3  
#define coefficient 4.8 
void setup()
{
    Serial.begin(115200);
    if (DIRECTION == ANGLE_0_DEGREE || DIRECTION == ANGLE_180_DEGREE)
    {
        width = display._width;
        height = display._height;
    }
    else
    {
        width = display._height;
        height = display._width;
    }
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
    int battery_levl = analogRead(7)* Resolution * battary_in * coefficient;//battary/4096*3.3* coefficient
    float battery_one = 0.4125;
    Serial.printf("ADC analog value = %.2f\n", battery_levl );
    if (battery_levl < battery_one)
    {
        display.drawString(230, 0, "N/A");
        display.drawXbm(255, 0, battery_w, battery_h, battery0);
    }
    else if (battery_levl < 2 * battery_one && battery_levl > battery_one)
    {
        display.drawXbm(270, 0, battery_w, battery_h, battery1);
    }
    else if (battery_levl < 3 * battery_one && battery_levl > 2 * battery_one)
    {
        display.drawXbm(270, 0, battery_w, battery_h, battery2);
    }
    else if (battery_levl < 4 * battery_one && battery_levl > 3 * battery_one)
    {
        display.drawXbm(270, 0, battery_w, battery_h, battery3);
    }
    else if (battery_levl < 5 * battery_one && battery_levl > 4 * battery_one)
    {
        display.drawXbm(270, 0, battery_w, battery_h, battery4);
    }
    else if (battery_levl < 6 * battery_one && battery_levl > 5 * battery_one)
    {
        display.drawXbm(270, 0, battery_w, battery_h, battery5);
    }
    else if (battery_levl < 7 * battery_one && battery_levl > 6 * battery_one)
    {
        display.drawXbm(270, 0, battery_w, battery_h, battery6);
    }
    else if (battery_levl < 7 * battery_one && battery_levl > 6 * battery_one)
    {
        display.drawXbm(270, 0, battery_w, battery_h, batteryfull);
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
    display.clear();
    display.drawLine(0, 15, 296, 15);
    battery();
    display.display();
}