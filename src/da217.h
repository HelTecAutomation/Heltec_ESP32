#ifndef __DA217_H__
#define __DA217_H__

#include "Arduino.h"
#include <Wire.h>


//#define ORIENT_HYST (2CH)

//DA217 registers start
// INT_SET0 0x15
#define Tilt_int_en 0x10
#define Watermark_int_en    0x08
#define FIFO_full_int_en    0x04
#define DA217_SM_INT_EN          (0x01 <<1)
#define DA217_STEP_INT_EN        (0x01 <<0)

//INT_SET0 Tilt_int_en | FIFO_full_int_en

#define INT_SET1 0x16
#define INT_source_mask 0xC0
#define INT_source_oversampling 00      //00: oversampling data (ODR_period =ODR*OSR)
#define INT_source_unfiltered   0x40    // 01: unfiltered data (ODR_period =ODR)
#define INT_source_filtered     0x80    // 10/11: filtered data (ODR_period =ODR IIR)
#define S_tap_int_en    0x20
#define D_tap_int_en    0x10
#define Orient_int_en   0x08
#define Active_int_en_z 0x04
#define Active_int_en_y 0x02
#define Active_int_en_x 0x01
//INT_SET1 INT_source_unfiltered | Orient_int_en

#define INT_SET2 0x17
#define Temporary_dis   0x80        // temporary disable all interrupts for a short time(configured by temp_dis_time)
#define Temp_dis_time_mask  0x60    
#define Temp_dis_time_100ms 0      //00://00:100ms
#define Temp_dis_time_1s    0x20    //01:1s
#define Temp_dis_time_2s    0x40    //10:2s
#define Temp_dis_time_4s    0x60    //11:4s
#define New_data_int_en 0x10
#define Freefall_int_en 0x08
// unused unused unused
// INT_SET2    0

// #define MOTION_FLAG 0x09
#define DA217_STEP_INT      (0x01 <<7)
#define DA217_ORIENT_INT    (0x01 <<6)
#define DA217_S_TAP_INT     (0x01 <<5)
#define DA217_D_TAP_INT     (0x01 <<4)
#define DA217_SM_INT        (0x01 <<3)
#define DA217_ACTIVE_INT    (0x01 <<2)
#define DA217_TILT_INT      (0x01 <<1)
#define DA217_FREEFALL_INT  (0x01 <<0)

#define Tilt_ths_value   0x10
#define TAP_THS     0x2B

#define INT_MAP1 0x19
#define DA217_INT1_SM       (0x01 <<7)
#define DA217_INT1_ORIENT   (0x01 <<6)
#define DA217_INT1_S_TAP    (0x01 <<5)
#define DA217_INT1_D_TAP    (0x01 <<4)
#define DA217_INT1_TILT     (0x01 <<3)
#define DA217_INT1_ACTIVE   (0x01 <<2)
#define DA217_INT1_STEP     (0x01 <<1)
#define DA217_INT1_FREEFALL (0x01 <<0)
// INT_MAP1 Int1_tilt | Int1_orient

#define INT_MAP2 0x1A
#define Int2_new_data   0x80
#define Int2_watermark  0x40
#define Int2_FIFO_full  0x20
//unused unused 
#define Int1_FIFO_full  0x04
#define Int1_watermark  0x02
#define Int1_new_data  0x01
//INT_MAP2    0 // Int2_FIFO_full

// #define INT_MAP3 0x1B
#define DA217_INT2_SM       (0x01 <<7)
#define DA217_INT2_ORIENT   (0x01 <<6)
#define DA217_INT2_S_TAP    (0x01 <<5)
#define DA217_INT2_D_TAP    (0x01 <<4)
#define DA217_INT2_TILT     (0x01 <<3)
#define DA217_INT2_ACTIVE   (0x01 <<2)
#define DA217_INT2_STEP     (0x01 <<1)
#define DA217_INT2_FREEFALL (0x01 <<0)
// INT_MAP3 0

#define INT_CONFIG 0x20
#define Reset_int   0x80
//unused unused unused 
#define Int2_od     0x08
#define Int2_lvl    0x04
#define Int1_od     0x02
#define Int1_lvl    0x01
// INT_CONFIG Reset_int

// #define INT_LATCH 0x21
// INT_LATCH 0x11 int2:int1 0001: temporary latched 250ms
#define DA217_INT_LATCH_VALUE 0x00


#define Z_BLOCK 0x2D

#define DA217_ORIENT_STATUS 0x0C
//unused 
#define OrientZ     0x40
#define OrientXY    0x30
//unused unused 
#define DA217_STEP_STATUS  0x03

#define OrientZ_upward_looking  0
#define OrientZ_downward_looking  OrientZ
#define OrientXY_portrait_upright   0
#define OrientXY_portrait_upside_down   0x10
#define OrientXY_landscape_left   0x20
#define OrientXY_landscape_right   0x30

#define Step_status_idle0   0
#define Step_status_idle1   3
#define Step_status_walk   1
#define Step_status_run   2

#define ORIENT_HYST 0x2C
//unused Orient_hyst[2] Orient_hyst[1] Orient_hyst[0] Orient_block[1] Orient_block [0] Orient_mode [1] Orient_mode [0]
#define Orient_hyst_level_0 0       // a blocking value between 0g and 0.9375g with an LSB = 0.0625g.
#define Orient_hyst_level_1 0x010
#define Orient_hyst_level_2 0x020
#define Orient_hyst_level_3 0x030
#define Orient_hyst_level_4 0x040
#define Orient_hyst_level_5 0x050
#define Orient_hyst_level_6 0x060
#define Orient_hyst_level_7 0x070
//Set the hysteresis of the orientation interrupt
//1LSB = 62.5mg.

#define Orient_block_no_blocking1    0
#define Orient_block_z_blocking    0x04
#define Orient_block_z_blocking_with_slope    0x08
#define Orient_block_no_blocking2    0x0c

#define Orient_mode_symmetrical 0
#define Orient_mode_high_asymmetrical 1
#define Orient_mode_low_asymmetrical 2
#define Orient_mode_synmmetrical 3

#define DA217_STEP_FILTER_EN (0X01 <<7)

//Step_en unused unused unused unused unused unused unused
#define DA217_RESET_STEP 0x80
//Reset_steps unused unused unused unused unused unused unused
// #define SM_THRESHOLD 0x34
// SM_threshold[7:0] 0-255 step


// #define RESOLUTION_RANGE 0x0F
//HP_en Wdt_en Wdt_time unused Resolution[1] Resolution[0] FS[1] FS[0]
#define HP_EN               (0x01 << 7)
#define WATCHDOG_EN        (0x01 << 6)
#define WATCHDOG_TIME_1MS  0x00
#define WATCHDOG_TIME_50MS (0x01 << 5)


//00: +/-2g
//01: +/-4g
//10:+/-8g
//11:+/-16

// #define ODR_AXIS 0x10
//X-axis_disable Y-axis_disable Z-axis_disable unused ODR[3] ODR[2] ODR[1] ODR[0]
#define DA217_X_AXIS_DIS  (0X01 <<7)
#define DA217_Y_AXIS_DIS  (0X01 <<6)
#define DA217_Z_AXIS_DIS  (0X01 <<5)


// #define MODE_BW 0x11
//PWR_OFF unused unused unused unused BW[1] BW[0] autosleep_en
#define DA217_PWR_ON        (0x00)

#define DA217_BANDWIDTH_500HZ  (0x00)
#define DA217_BANDWIDTH_250HZ  (0x01 <<1)
#define DA217_BANDWIDTH_100HZ  (0x01 <<2)
//00/11:500hz
//01:250
//10:100
#define DA217_AUTOSLEEP_EN     (0x01 <<0)

//0: working the current ODR state all the way
//1: Working at 12.5hz in inactive state, automatic switched to normal mode during active state

#define SWAP_POLARITY 0x12
#define ACC_Z_LSB 0x06
#define ACC_Z_MSB 0x07
#define FIFO_STATUS 0x08

// #define FIFO_CTRL 0x14
/*
FIFO_mode[1] FIFO_mode[0] unused
Watermark_ 
samples[4]
Watermark_ 
samples[3]
Watermark_ 
samples[2]
Watermark_ 
samples[1]
Watermark_ 
samples[0]
*/

#define Watermark_samples_31    0x1F

#define NEWDATA_FLAG 0x0A

#define TAP_ACTIVE_STATUS 0x0B

#define ACTIVE_DUR 0x27
//Inactive_dur[3] Inactive_dur[2] Inactive_dur[1] Inactive_dur[0] Active_dur[3] Active_dur[2] Active_dur[1] Active_dur[0]
#define Inactive_dur_4  0x40    // inactive duration time = (Inactive_dur + 1)* ODR_period
#define Inactive_dur_8  0x80    // inactive duration time = (Inactive_dur + 1)* ODR_period
#define Active_dur_4    0x04    // Active duration time = (Active_dur + 1)* ODR_period

#define ACTIVE_THS 0x28
//Active_th[7] Active_th [6] Active_th [5] Active_th[4] Active_th [3] Active_th [2] Active_th [1] Active_th [0]
#define Active_th_20    0x20
//Threshold of active interrupt=Active_th*K(mg)
//K = 3.91(2g range),
//K =7.81(4g range),
//K=15.625(8g range), 
//K=31.25(16g range)
//DA217 registers end

/*******************************************************************************
Macro definitions - Register define for Gsensor asic
********************************************************************************/
#define DA217_REG_SPI_I2C                 0x00
#define DA217_REG_WHO_AM_I                0x01
#define DA217_REG_ACC_X_LSB               0x02
#define DA217_REG_ACC_X_MSB               0x03
#define DA217_REG_ACC_Y_LSB               0x04
#define DA217_REG_ACC_Y_MSB               0x05
#define DA217_REG_ACC_Z_LSB               0x06
#define DA217_REG_ACC_Z_MSB               0x07 
#define DA217_REG_MOTION_FLAG				0x09
#define DA217_REG_STEPS_MSB				0x0D
#define DA217_REG_STEPS_LSB				0x0E
#define DA217_REG_RESOLUTION_RANGE      0x0F
#define DA217_REG_ODR_AXIS        0x10
#define DA217_REG_MODE_BW            0x11
#define DA217_REG_SWAP_POLARITY           0x12
#define DA217_REG_FIFO_CTRL               0x14
#define DA217_REG_INT_SET0				 0x15
#define DA217_REG_INTERRUPT_SETTINGS1     0x16
#define DA217_REG_INTERRUPT_SETTINGS2     0x17
#define DA217_REG_INTERRUPT_MAPPING1      0x19
#define DA217_REG_INTERRUPT_MAPPING2      0x1a
#define DA217_REG_INTERRUPT_MAPPING3      0x1b
#define DA217_REG_INT_PIN_CONFIG          0x20
#define DA217_REG_INT_LATCH               0x21
#define DA217_REG_ACTIVE_DURATION         0x27
#define DA217_REG_ACTIVE_THRESHOLD        0x28
#define DA217_REG_TAP_DURATION            0x2A
#define DA217_REG_TAP_THRESHOLD           0x2B
#define DA217_REG_RESET_STEP             0x2E
#define DA217_REG_STEP_CONFIG1			0x2F
#define DA217_REG_STEP_CONFIG2			0x30
#define DA217_REG_STEP_CONFIG3			0x31
#define DA217_REG_STEP_CONFIG4			0x32
#define DA217_REG_STEP_FILTER		     0x33
#define DA217_REG_SM_THRESHOLD			0x34


#define I2C_ADDRESS 0x26

typedef enum 
{
	IDLE1 = 0x00,
	WALK  = 0x01,
	RUN   = 0x10,
	IDLE2 = 0x11,
}da217_step_status_t;


typedef enum 
{
	FIFO_MODE_BYPASS  = 0x00,
	FIFO_MODE_FIFO    = 0x40,
	FIFO_MODE_STREAM  = 0x80,
	FIFO_MODE_TRIGGER = 0xc0,
}da217_fifo_mode_t;

typedef enum
{
	FS_FULL_SCALE_2g  =   0,
	FS_FULL_SCALE_4g  =   1,
	FS_FULL_SCALE_8g  =   2,
	FS_FULL_SCALE_16g =   3
}da217_full_scale_t;

typedef enum 
{
	RESOLUTION_14_BIT  =   0,
	RESOLUTION_12_BIT  =   4,
	RESOLUTION_10_BIT  =   8,
	RESOLUTION_8_BIT =   0x0c
}da217_resolution_t;


typedef enum 
{
	DA217_ODR_1HZ  =0,
	DA217_ODR_2HZ  =1,
	DA217_ODR_4HZ  =2,
	DA217_ODR_8HZ  =3,
	DA217_ODR_16HZ =4,
	DA217_ODR_32HZ =5,
	DA217_ODR_63HZ =6,
	DA217_ODR_125HZ =7,
	DA217_ODR_250HZ =8,
	DA217_ODR_500HZ =9,
}da217_odr_rate_t;
class DA217 
{
public:
	void da217_init( int sda=1,int scl=2 ,uint32_t freq=50000);
	void da217_start_up_step_detect(void);
	void da217_stop_step_detect(void);
	uint16_t da217_read_steps(void);
	da217_step_status_t da217_read_step_status(void);

	void da217_poweron(void);
	void da217_poweroff(void);
	void da217_enable_watchdog(void);
	void da217_disable_watchdog(void);
	void da217_start_xyz_axis(bool x_en,bool y_en,bool z_en);
	void da217_set_odr_rate(da217_odr_rate_t odr_rate);
	void da217_set_fifo_mode(da217_fifo_mode_t mode);
	void da217_set_full_scale(da217_full_scale_t fs);
	void da217_set_resolution(da217_resolution_t resolution);
	void da217_read_xyz_data(int16_t *x_data,int16_t *y_data,int16_t *z_data);
	void da217_gravity_init(void);
	void da217_get_xyz_gravity(float *x_gravity_g, float *y_gravity_g,float *z_gravity_g);
	void da217_get_vector_sum_gravity(float *vector_sum);

private:
	uint8_t             _address;
	int                 _sda;
	int                 _scl;
	uint32_t            _freq;
	void da217_write_single_reg(uint8_t reg,uint8_t data);
	void da217_write_multiple_reg(uint8_t reg_begin,uint8_t *data,uint8_t data_size);
	uint8_t da217_read_single_reg(uint8_t reg);
	void da217_read_multiple_reg(uint8_t reg_begin,uint8_t* data,uint8_t data_size);
};

#endif

