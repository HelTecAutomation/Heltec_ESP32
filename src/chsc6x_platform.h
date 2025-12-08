#ifndef __CHSC6X_PLATFORM_H__
#define __CHSC6X_PLATFORM_H__
//#include "printf.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

#define OS_OK                    (0)
#define OS_ERROR                 (1)
#define TP_RETRY_CNT             (2)
#define TP_RETRY_CNT2            (3)
#define TP_RETRY_CNT3            (5)

#if 0
#define chsc6x_info Serial.printf
#define chsc6x_err Serial.printf
#else
#define chsc6x_info       
#define chsc6x_err      
#endif



#define CHSC6X_I2C_ID      (0x2E)  //8bit 

#define CHSC6X_MAX_POINTS_NUM     (2)

/*MACRO SWITCH for TP gesture function */
#define CHSC6X_GESTURE                (0)

/*MACRO SWITCH for Proximity function */
#define CHSC6X_PROXIMITY              (0)

/*MACRO SWITCH for esd check function */
#define CHSC6X_ESD_CHECK              (0)

#define MAX_IIC_WR_LEN          (32)
#define MAX_IIC_RD_LEN          (32)

typedef enum
{
    RESET_NONE,
    HW_CMD_RESET,
    HW_ACTIVE_RESET,
    RESET_MAX
} chsc6x_reset_e;
class chsc6x_platform
{
private:
    TwoWire *_wire;
    int _sda_pin;
    int _scl_pin;
    int _int_pin;
    int _rst_pin;
public:
    uint8_t _i2c_addr;
    chsc6x_platform(TwoWire *wire = &Wire1,int sda_pin=-1,int scl_pin=-1,int int_pin=-1,int rst_pin=-1) {
        this->_wire = wire;
        this->_sda_pin = sda_pin;
        this->_scl_pin = scl_pin;
        this->_int_pin = int_pin;
        this->_rst_pin = rst_pin;
        this->_i2c_addr = CHSC6X_I2C_ID;
    };
    ~chsc6x_platform(){};

/* fail : <0 */
int i2cRead(uint8_t i2c_adr, uint16_t reg_adr, uint8_t *rxbuf, uint16_t lenth);
int i2cSend(uint8_t i2c_adr, uint16_t reg_adr, uint8_t *txbuf, uint16_t lenth);

/* RETURN:0->pass else->fail */
int chsc6x_read_bytes_u8addr(uint8_t id, uint8_t adr, uint8_t *rxbuf, uint16_t lenth);
/* RETURN:0->pass else->fail */
int chsc6x_read_bytes_u16addr(uint8_t id, uint16_t adr, uint8_t *rxbuf, uint16_t lenth);

/* RETURN:0->pass else->fail */
int chsc6x_write_bytes_u16addr(uint8_t id, uint16_t addr, uint8_t *txbuf, uint16_t lenth);
void chsc6x_tp_reset_delay(chsc6x_reset_e type);

void chsc6x_msleep(int ms);
int chsc6x_tp_reset(chsc6x_reset_e type);
void semi_rst_pin_low(int pin);
void semi_rst_pin_high(int pin);
int semi_touch_get_int(void);
int semi_touch_get_rst(void);
// extern struct sm_touch_dev st_dev;
int semi_touch_i2c_init(void);
};
#endif
