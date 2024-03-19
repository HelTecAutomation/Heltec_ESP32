

#ifndef _HELTEC_H_
#define _HELTEC_H_

#if defined(ESP32)

#include <Arduino.h>

/* Define board classes */
#if defined( WIFI_Kit_32 ) || defined( WIFI_Kit_32_V3 )
#define Class_Wifi_Kit
#endif

#if defined( WIFI_LORA_32 ) || defined( WIFI_LORA_32_V2 ) || defined( WIFI_LORA_32_V3 )
#define Class_WIFI_LORA
#endif

#if defined( WIRELESS_STICK ) || defined( WIRELESS_STICK_LITE ) || defined( WIRELESS_STICK_V3 ) || defined( WIRELESS_STICK_LITE_V3 )
#define Class_WIRELESS_STICK
#endif


/* Define board capabilities */
#if defined( Class_Wifi_Kit ) || defined( Class_WIFI_LORA ) || defined( Class_WIRELESS_STICK ) || defined( WIRELESS_BRIDGE )
#define Heltec_Wifi
#endif

#if defined( Class_WIFI_LORA ) || defined( Class_WIRELESS_STICK ) || defined( WIRELESS_BRIDGE )
#define Heltec_LoRa
#endif

#if defined( Class_Wifi_Kit ) || defined( Class_WIFI_LORA ) || defined( WIRELESS_STICK )
#define Heltec_Screen
#endif

/* wifi kit 32 and WiFi LoRa 32(V1) do not have vext */
#if defined( WIFI_Kit_32_V3 ) || defined( WIFI_LORA_32_V2 ) || defined( WIFI_LORA_32_V3 ) || defined( Class_WIRELESS_STICK ) || defined( WIRELESS_BRIDGE )
#define Heltec_Vext
#endif


#ifdef Heltec_Screen
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#endif

#ifdef Heltec_LoRa
	#include <SPI.h>
	#include "lora/LoRa.h"
#endif


class Heltec_ESP32 {

 public:
    Heltec_ESP32();
	~Heltec_ESP32();

    void begin(bool DisplayEnable=true, bool LoRaEnable=true, bool SerialEnable=true, bool PABOOST=true, long BAND=470E6);
#ifdef Heltec_LoRa
    LoRaClass LoRa;
#endif

#ifdef Heltec_Screen
    SSD1306Wire *display;
#endif

/*wifi kit 32 and WiFi LoRa 32(V1) do not have vext*/
#ifdef Heltec_Vext
    void VextON(void);
    void VextOFF(void);
#endif
};

extern Heltec_ESP32 Heltec;

#else
#error "This library only supports boards with ESP32 processor."
#endif


#endif
