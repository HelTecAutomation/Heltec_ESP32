/*!
 *  @file BMP280.cpp
 *
 *  This is a library for the BMP280 orientation sensor
 *
 *  Designed specifically to work with the Adafruit BMP280 Sensor.
 *
 *  Pick one up today in the adafruit shop!
 *  ------> https://www.adafruit.com/product/2651
 *
 *  These sensors use I2C to communicate, 2 pins are required to interface.
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 *
 *  K.Townsend (Adafruit Industries)
 *
 *  BSD license, all text above must be included in any redistribution
 *
 * modified to work with the HelTec CubeCell Series
 */

#include "BMP280.h"

/*!
 * @brief  BMP280 constructor using i2c
 * @param  *theWire
 *         optional wire
 */
BMP280::BMP280(TwoWire *theWire)
    : _cs(-1), _mosi(-1), _miso(-1), _sck(-1) {
  _wire = theWire;
}

/*!
 * @brief  BMP280 constructor using hardware SPI
 * @param  cspin
 *         cs pin number
 * @param  theSPI
 *         optional SPI object
 */
/* Adafruit_BMP280::Adafruit_BMP280(int8_t cspin, SPIClass *theSPI)
    : _cs(cspin), _mosi(-1), _miso(-1), _sck(-1) {
  _spi = theSPI;
} */

/*!
 * @brief  BMP280 constructor using bitbang SPI
 * @param  cspin
 *         The pin to use for CS/SSEL.
 * @param  mosipin
 *         The pin to use for MOSI.
 * @param  misopin
 *         The pin to use for MISO.
 * @param  sckpin
 *         The pin to use for SCK.
 */
/* Adafruit_BMP280::Adafruit_BMP280(int8_t cspin, int8_t mosipin, int8_t misopin,
                                 int8_t sckpin)
    : _cs(cspin), _mosi(mosipin), _miso(misopin), _sck(sckpin) {}
 */
/*!
 *  Initialises the sensor.
 *  @param addr
 *         The I2C address to use (default = 0x77)
 *  @param chipid
 *         The expected chip ID (used to validate connection).
 *  @return True if the init was successful, otherwise false.
 */
bool BMP280::begin(uint8_t addr, uint8_t chipid) {
  _i2caddr = addr;

  if (_cs == -1) {
    // i2c
    _wire->begin();
  } else {
    digitalWrite(_cs, HIGH);
    pinMode(_cs, OUTPUT);

    if (_sck == -1) {
      // hardware SPI
      _spi->begin();
    } else {
      // software SPI
      pinMode(_sck, OUTPUT);
      pinMode(_mosi, OUTPUT);
      pinMode(_miso, INPUT);
    }
  }

  if (read8(BMP280_REGISTER_CHIPID) != chipid)
    return false;

  readCoefficients();
  // write8(BMP280_REGISTER_CONTROL, 0x3F); /* needed? */
  setSampling();
  delay(100);
  return true;
}

/*!
 * Sets the sampling config for the device.
 * @param mode
 *        The operating mode of the sensor.
 * @param tempSampling
 *        The sampling scheme for temp readings.
 * @param pressSampling
 *        The sampling scheme for pressure readings.
 * @param filter
 *        The filtering mode to apply (if any).
 * @param duration
 *        The sampling duration.
 */
void BMP280::setSampling(sensor_mode mode,
                                  sensor_sampling tempSampling,
                                  sensor_sampling pressSampling,
                                  sensor_filter filter,
                                  standby_duration duration) {
  _measReg.mode = mode;
  _measReg.osrs_t = tempSampling;
  _measReg.osrs_p = pressSampling;

  _configReg.filter = filter;
  _configReg.t_sb = duration;

  write8(BMP280_REGISTER_CONFIG, _configReg.get());
  write8(BMP280_REGISTER_CONTROL, _measReg.get());
}

/* uint8_t Adafruit_BMP280::spixfer(uint8_t x) {
  if (_sck == -1)
    return _spi->transfer(x);

  // software spi
  // Serial.println("Software SPI");
  uint8_t reply = 0;
  for (int i = 7; i >= 0; i--) {
    reply <<= 1;
    digitalWrite(_sck, LOW);
    digitalWrite(_mosi, x & (1 << i));
    digitalWrite(_sck, HIGH);
    if (digitalRead(_miso))
      reply |= 1;
  }
  return reply;
} */

/**************************************************************************/
/*!
    @brief  Writes an 8 bit value over I2C/SPI
*/
/**************************************************************************/
void BMP280::write8(byte reg, byte value) {
  if (_cs == -1) {
    _wire->beginTransmission((uint8_t)_i2caddr);
    _wire->write((uint8_t)reg);
    _wire->write((uint8_t)value);
    _wire->endTransmission();
  } else {
    /* if (_sck == -1)
      _spi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg & ~0x80); // write, bit 7 low
    spixfer(value);
    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      _spi->endTransaction(); // release the SPI bus */
  }
}

/*!
 *  @brief  Reads an 8 bit value over I2C/SPI
 *  @param  reg
 *          selected register
 *  @return value from selected register
 */
uint8_t BMP280::read8(byte reg) {
  uint8_t value;

  if (_cs == -1) {
    _wire->beginTransmission((uint8_t)_i2caddr);
    _wire->write((uint8_t)reg);
    _wire->endTransmission();
    _wire->requestFrom((uint8_t)_i2caddr, (byte)1);
    value = _wire->read();

  } else {
    /* if (_sck == -1)
      _spi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high
    value = spixfer(0);
    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      _spi->endTransaction(); // release the SPI bus */
  }
  return value;
}

/*!
 *  @brief  Reads a 16 bit value over I2C/SPI
 */
uint16_t BMP280::read16(byte reg) {
  uint16_t value;

  if (_cs == -1) {
    _wire->beginTransmission((uint8_t)_i2caddr);
    _wire->write((uint8_t)reg);
    _wire->endTransmission();
    _wire->requestFrom((uint8_t)_i2caddr, (byte)2);
    value = (_wire->read() << 8) | _wire->read();

  } else {
    /* if (_sck == -1)
      _spi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high
    value = (spixfer(0) << 8) | spixfer(0);
    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      _spi->endTransaction(); // release the SPI bus */
  }

  return value;
}

uint16_t BMP280::read16_LE(byte reg) {
  uint16_t temp = read16(reg);
  return (temp >> 8) | (temp << 8);
}

/*!
 *   @brief  Reads a signed 16 bit value over I2C/SPI
 */
int16_t BMP280::readS16(byte reg) { return (int16_t)read16(reg); }

int16_t BMP280::readS16_LE(byte reg) {
  return (int16_t)read16_LE(reg);
}

/*!
 *  @brief  Reads a 24 bit value over I2C/SPI
 */
uint32_t BMP280::read24(byte reg) {
  uint32_t value;

  if (_cs == -1) {
    _wire->beginTransmission((uint8_t)_i2caddr);
    _wire->write((uint8_t)reg);
    _wire->endTransmission();
    _wire->requestFrom((uint8_t)_i2caddr, (byte)3);

    value = _wire->read();
    value <<= 8;
    value |= _wire->read();
    value <<= 8;
    value |= _wire->read();

  } else {
   /*  if (_sck == -1)
      _spi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high

    value = spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    value <<= 8;
    value |= spixfer(0);

    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      _spi->endTransaction(); // release the SPI bus */
  }

  return value;
}

/*!
 *  @brief  Reads the factory-set coefficients
 */
void BMP280::readCoefficients() {
  _bmp280_calib.dig_T1 = read16_LE(BMP280_REGISTER_DIG_T1);
  _bmp280_calib.dig_T2 = readS16_LE(BMP280_REGISTER_DIG_T2);
  _bmp280_calib.dig_T3 = readS16_LE(BMP280_REGISTER_DIG_T3);

  _bmp280_calib.dig_P1 = read16_LE(BMP280_REGISTER_DIG_P1);
  _bmp280_calib.dig_P2 = readS16_LE(BMP280_REGISTER_DIG_P2);
  _bmp280_calib.dig_P3 = readS16_LE(BMP280_REGISTER_DIG_P3);
  _bmp280_calib.dig_P4 = readS16_LE(BMP280_REGISTER_DIG_P4);
  _bmp280_calib.dig_P5 = readS16_LE(BMP280_REGISTER_DIG_P5);
  _bmp280_calib.dig_P6 = readS16_LE(BMP280_REGISTER_DIG_P6);
  _bmp280_calib.dig_P7 = readS16_LE(BMP280_REGISTER_DIG_P7);
  _bmp280_calib.dig_P8 = readS16_LE(BMP280_REGISTER_DIG_P8);
  _bmp280_calib.dig_P9 = readS16_LE(BMP280_REGISTER_DIG_P9);
}

/*!
 * Reads the temperature from the device.
 * @return The temperature in degress celcius.
 */
float BMP280::readTemperature() {
  int32_t var1, var2;

  int32_t adc_T = read24(BMP280_REGISTER_TEMPDATA);
  adc_T >>= 4;

  var1 = ((((adc_T >> 3) - ((int32_t)_bmp280_calib.dig_T1 << 1))) *
          ((int32_t)_bmp280_calib.dig_T2)) >>
         11;

  var2 = (((((adc_T >> 4) - ((int32_t)_bmp280_calib.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)_bmp280_calib.dig_T1))) >>
           12) *
          ((int32_t)_bmp280_calib.dig_T3)) >>
         14;

  t_fine = var1 + var2;

  float T = (t_fine * 5 + 128) >> 8;
  return T / 100;
}

/*!
 * Reads the barometric pressure from the device.
 * @return Barometric pressure in hPa.
 */
float BMP280::readPressure() {
  int64_t var1, var2, p;

  // Must be done first to get the t_fine variable set up
  readTemperature();

  int32_t adc_P = read24(BMP280_REGISTER_PRESSUREDATA);
  adc_P >>= 4;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)_bmp280_calib.dig_P6;
  var2 = var2 + ((var1 * (int64_t)_bmp280_calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)_bmp280_calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)_bmp280_calib.dig_P3) >> 8) +
         ((var1 * (int64_t)_bmp280_calib.dig_P2) << 12);
  var1 =
      (((((int64_t)1) << 47) + var1)) * ((int64_t)_bmp280_calib.dig_P1) >> 33;

  if (var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)_bmp280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)_bmp280_calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)_bmp280_calib.dig_P7) << 4);
  return (float)p / 256;
}

/*!
 * @brief Calculates the approximate altitude using barometric pressure and the
 * supplied sea level hPa as a reference.
 * @param seaLevelhPa
 *        The current hPa at sea level.
 * @return The approximate altitude above sea level in meters.
 */
float BMP280::readAltitude(float seaLevelhPa) {
  float altitude;

  float pressure = readPressure(); // in Si units for Pascal
  pressure /= 100;

  altitude = 44330 * (1.0 - pow(pressure / seaLevelhPa, 0.1903));

  return altitude;
}

/*!
 * Calculates the pressure at sea level (in hPa) from the specified altitude
 * (in meters), and atmospheric pressure (in hPa).
 * @param  altitude      Altitude in meters
 * @param  atmospheric   Atmospheric pressure in hPa
 * @return The approximate pressure
 */
float BMP280::seaLevelForAltitude(float altitude, float atmospheric) {
  // Equation taken from BMP180 datasheet (page 17):
  // http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  // http://forums.adafruit.com/viewtopic.php?f=22&t=58064
  return atmospheric / pow(1.0 - (altitude / 44330.0), 5.255);
}

/*!
 *  @brief  Take a new measurement (only possible in forced mode)
 *  !!!todo!!!
 */
/*
void BMP280::takeForcedMeasurement()
{
    // If we are in forced mode, the BME sensor goes back to sleep after each
    // measurement and we need to set it to forced mode once at this point, so
    // it will take the next measurement and then return to sleep again.
    // In normal mode simply does new measurements periodically.
    if (_measReg.mode == MODE_FORCED) {
        // set to forced mode, i.e. "take next measurement"
        write8(BMP280_REGISTER_CONTROL, _measReg.get());
        // wait until measurement has been completed, otherwise we would read
        // the values from the last measurement
        while (read8(BMP280_REGISTER_STATUS) & 0x08)
                delay(1);
    }
}
*/
