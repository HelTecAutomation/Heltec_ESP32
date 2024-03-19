#ifndef McuSet_H
#define McuSet_H

#include <Arduino.h>
#include "SPI.h"
#include "driver/rtc-board.h"
#include "driver/board-config.h"
#include "driver/lorawan_spi.h"
#include "driver/sx126x.h"

/* HELTEC_BOARD
*
* ESP32S3 CHIP
* WIFI_LORA_32_V3          30
* WIRELESS_STICK_V3        31
* WIRELESS_STICK_LITE_V3   32
* WIRELESS_SHELL_V3        33
* WIRELESS_TRACKER         34
* WIFI_KIT_32_V3           35
*
* CAPSULE_SENSOR_V3 50
* WIRELESS_PAPER    60
*
*
* ESP32C3 CHIP
* WIRELESS_MINI_SHELL      70
*
*
* ESP32D0 CHIP
*
* WIFI_LORA_32_V2         0
* WIFI_LORA_32            1
* WIRELESS_STICK          2
* WIRELESS_STICK_LITE //WIRELESS_SHELL     3
* WIRELESS_BRIDGE         4
* WIFI_KIT_32             5
*/



#define LORA_DEFAULT_NSS_PIN    18
#define LORA_DEFAULT_RESET_PIN  14
#define LORA_DEFAULT_DIO0_PIN   26
#define LORA_DEFAULT_DIO1_PIN   33
#define Timer_DEFAULT_DIV       80
extern uint8_t mcuStarted;
class McuClass{
public:
  McuClass();
  void setlicense(uint32_t * license,uint8_t board_type);
  int begin(uint8_t board_type,uint8_t ex_32k);
  void addwakeio(uint8_t gpio);
  void setSPIFrequency(uint32_t frequency);
  void timerhandler();
  void sleep(uint8_t classMode,uint8_t debugLevel,uint8_t board_type,uint8_t ex_32k);
  SPISettings _spiSettings;
private:

};
extern TimerEvent_t TxNextPacketTimer;

#ifdef __cplusplus
extern "C" uint8_t SpiInOut(Spi_t *obj, uint8_t outData );
extern "C" uint64_t timercheck();
extern "C" uint64_t getID();
extern "C" void SX126xIoInit( void );
extern "C" void SX126xIoIrqInit( DioIrqHandler dioIrq );
extern "C" void RadioOnDioIrq( void );
extern "C" void SX126xIoInit( void );
extern "C" void SX126xReset( void );
extern "C" void sx126xSleep( void );
extern "C"  void calrtc();
extern "C" void lora_printf(const char *format, ...);
#ifdef RADIO_CHIP_SX127X
extern "C" void SX1276IoInit( void );
extern "C" void SX1276IoIrqInit( DioIrqHandler **irqHandlers );
extern DioIrqHandler *DioIrq[];
#endif

#endif

extern McuClass Mcu;
#endif
