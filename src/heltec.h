

#ifndef _HELTEC_H_
#define _HELTEC_H_

#if defined(ESP32)

#include <Arduino.h>
#include <Wire.h>

#include "oled/SSD1306Wire.h"
#if !defined(WIFI_Kit_32)
	#include <SPI.h>
	#include "lora/LoRa.h"
#endif




class Heltec_ESP32 {

 public:
    Heltec_ESP32();
	~Heltec_ESP32();

    void begin(bool DisplayEnable=true, bool LoRaEnable=true, bool SerialEnable=true, bool PABOOST=true, long BAND=470E6);
#if defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( Wireless_Stick )
    LoRaClass LoRa;
#endif

///////add by lxyzn of SSD1306DrawingDemo//////
#if defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( WIFI_Kit_32 )
    
   #define DISPLAY_HEIGHT 64
   #define DISPLAY_WIDTH  128
#endif

    SSD1306Wire *display;

     //wifi kit 32 no vext////
    void VextON(void);
    void VextOFF(void);
};

extern Heltec_ESP32 Heltec;

#else
#error ¡°This library only supports boards with ESP32 processor.¡±
#endif


#endif
