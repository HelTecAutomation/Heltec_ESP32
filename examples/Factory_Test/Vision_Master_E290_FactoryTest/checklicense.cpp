#include "LoRaWan_APP.h"


extern void getLicenseAddress(uint8_t board_type);
RTC_DATA_ATTR static bool licenseok=false;
extern void readlicense(uint8_t board_type);
extern uint32_t storedlicense[4];
extern void writelicense(uint32_t * license,uint8_t board_type);
extern int calRTC(uint32_t * license);
extern void check_input_license();
// extern uint32_t ex_32k_start_cnt;

void checklicense()
{
  if(licenseok==false)
  {
    getLicenseAddress(HELTEC_BOARD);
    readlicense(HELTEC_BOARD);
    if(calRTC(storedlicense)!=1)
    {
      uint64_t chipid=ESP.getEfuseMac();
      lora_printf("Please provide a correct license! For more information:\r\n");
      lora_printf("http://www.heltec.cn/search/\r\n");
      lora_printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
      lora_printf("%08X\r\n",(uint32_t)chipid);//print Low 4bytes.

      //read cdkey from uart
      uint32_t print_timestart;
      print_timestart = millis();

      for(;;)
      {
      if(millis() -print_timestart > 3000 )
      {
        print_timestart = millis();
        lora_printf("Please provide a correct license! For more information:\r\n");
        lora_printf("http://www.heltec.cn/search/\r\n");
        lora_printf("ESP32ChipID=%04X",(uint16_t)(chipid>>32));//print High 2 bytes
        lora_printf("%08X\r\n",(uint32_t)chipid);//print Low 4bytes.
      }
      
      check_input_license();
      if(calRTC(storedlicense)==1)
      {
        writelicense(storedlicense,HELTEC_BOARD);
        lora_printf("The board is actived\r\n");
        break;
      }
     }
    }
    else
    {
      licenseok=true;
    }
  }
}