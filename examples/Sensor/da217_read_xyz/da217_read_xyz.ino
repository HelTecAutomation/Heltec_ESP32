/* Heltec Automation  da217 Sensors test example
 *
 * Function:
 * Three axis acceleration sensor
 * 
 * HelTec AutoMation, Chengdu, China
 * www.heltec.org
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP32
 * 
*/

#include "Arduino.h"
#include "da217.h"

DA217 da217;
uint16_t step_num;
float x_data,y_data,z_data,sum_gravity;
void setup()
{
  Serial.begin(115200);

  da217.da217_gravity_init();
}

void loop()
{ 
    da217.da217_get_xyz_gravity(&x_data,&y_data,&z_data);
    da217.da217_get_vector_sum_gravity(&sum_gravity);
    Serial.printf("x_data=%f  y_data=%f z_data=%f  %f\r\n",x_data,y_data,z_data,sum_gravity);
    delay(3000);
}