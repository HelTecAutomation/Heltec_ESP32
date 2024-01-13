#ifndef __BH1750_H__
#define __BH1750_H__


#include <Arduino.h>
#include "Wire.h"


#define BH1750_SDA   (1)
#define BH1750_SCL   (2)
#define BH1750_RST   (6)
#define BH1750_AVERAGE_NUM (5)

// Uncomment, to enable debug messages
// #define BH1750_DEBUG

// No active state
#define BH1750_POWER_DOWN 0x00

// Waiting for measurement command
#define BH1750_POWER_ON 0x01

// Reset data register value - not accepted in POWER_DOWN mode
#define BH1750_RESET 0x07

// Default MTreg value
#define BH1750_DEFAULT_MTREG 69

class BH1750 {

  public:

    enum Mode
    {
      UNCONFIGURED = 0,
      // Measurement at 1 lux resolution. Measurement time is approx 120ms.
      CONTINUOUS_HIGH_RES_MODE  = 0x10,
      // Measurement at 0.5 lux resolution. Measurement time is approx 120ms.
      CONTINUOUS_HIGH_RES_MODE_2 = 0x11,
      // Measurement at 4 lux resolution. Measurement time is approx 16ms.
      CONTINUOUS_LOW_RES_MODE = 0x13,
      // Measurement at 1 lux resolution. Measurement time is approx 120ms.
      ONE_TIME_HIGH_RES_MODE = 0x20,
      // Measurement at 0.5 lux resolution. Measurement time is approx 120ms.
      ONE_TIME_HIGH_RES_MODE_2 = 0x21,
      // Measurement at 4 lux resolution. Measurement time is approx 16ms.
      ONE_TIME_LOW_RES_MODE = 0x23
    };

    BH1750(byte addr = 0x23);
    bool begin(int sda=BH1750_SDA, int scl=BH1750_SCL,int rst=BH1750_RST,Mode mode = CONTINUOUS_HIGH_RES_MODE);
    void end();
    bool configure(Mode mode);
    bool setMTreg(byte MTreg);
    float readLightLevel(bool maxWait = false);

  private:
    int _rst;
    byte BH1750_I2CADDR;
    byte BH1750_MTreg = (byte)BH1750_DEFAULT_MTREG;
    // Correction factor used to calculate lux. Typical value is 1.2 but can
    // range from 0.96 to 1.44. See the data sheet (p.2, Measurement Accuracy)
    // for more information.
    const float BH1750_CONV_FACTOR = 1.2;
    Mode BH1750_MODE = UNCONFIGURED;
};

#endif
