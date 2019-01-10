

#ifndef _HELTEC_H_
#define _HELTEC_H_

#if defined(ESP32)

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "oled/SSD1306Wire.h"
#include "lora/LoRa.h"


class Heltec_ESP32 {

 public:
    Heltec_ESP32();
	~Heltec_ESP32();

    void begin(bool DisplayEnable=true, bool LoRaEnable=true, bool SerialEnable=true, bool PABOOST=true, long BAND=470E6);

    LoRaClass LoRa;

    SSD1306Wire *display;

    void VextON(void);
    void VextOFF(void);
};

extern Heltec_ESP32 Heltec;

#else
#error ¡°This library only supports boards with ESP32 processor.¡±
#endif


#endif
