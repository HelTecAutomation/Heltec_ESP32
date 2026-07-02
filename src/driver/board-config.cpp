#include "ESP32_Mcu.h"

extern "C" McuRadioPins_t McuGetRadioPins(void)
{
  return McuRadioPins_t{
    RADIO_DIO_1,
    RADIO_NSS,
    RADIO_RESET,
    LORA_DEFAULT_BUSY_PIN,
    LORA_CLK,
    LORA_MISO,
    LORA_MOSI
  };
}