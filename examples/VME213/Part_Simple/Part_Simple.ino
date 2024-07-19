/* Heltec Automation Eink partial refresh example
 *
 * Function:
 * 1.Ink screen partial refresh demonstration.
 *
 * Description:
 * 1.Using an external Lut waveform file to achieve partial refresh
 * 
 * Note:
 * This example is compatible with Wireless Paper and requires you to change the pin Settings in your code and change the order in which the VextCtrl pin HIGH and LOW.
 * Library url: https://github.com/HelTecAutomation/Heltec_ESP32
 * Support: support@heltec.cn
 *
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * https://www.heltec.org
 * */
#include "HT_lCMEN2R13EFC1.h"
#include "images.h"
// Initialize the display
HT_ICMEN2R13EFC1 display(3, 2, 5, 1, 4, 6, -1, 6000000); // rst,dc,cs,busy,sck,mosi,miso,frequency
uint8_t num = 0;
typedef void (*Demo)(void);

/* screen rotation
 * ANGLE_0_DEGREE
 * ANGLE_90_DEGREE
 * ANGLE_180_DEGREE
 * ANGLE_270_DEGREE
 */
#define DIRECTION ANGLE_0_DEGREE

int width, height;
int demoMode = 0;

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

void part_ref(uint8_t _num)
{
  switch (_num)
  {
  case 0:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num0); // dis_img_Partial_Refresh(x,y,img_width,img_lenth)
                                                           // The second parameter, the fourth parameter, takes an integer multiple of 8
    break;
  case 1:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num1);
    break;
  case 2:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num2);
    break;
  case 3:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num3);
    break;
  case 4:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num4);
    break;
  case 5:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num5);
    break;
  case 6:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num6);
    break;
  case 7:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num7);
    break;
  case 8:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num8);
    break;
  case 9:
    display.dis_img_Partial_Refresh(100, 32, 14, 4, num9);
    break;
  default:
    break;
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
  part_ref(num++);
}
