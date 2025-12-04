#ifndef __CHSC6X_MAIN_H__
#define __CHSC6X_MAIN_H__
#include <stdint.h>
#include "chsc6x_platform.h"

typedef struct sm_touch_dev
{    
    int int_pin;
    int rst_pin;
}sm_touch_dev, *psm_touch_dev;

struct ts_event {
     uint16_t x; /*x coordinate */
     uint16_t y; /*y coordinate */
    int flag; /* touch event flag: 0 -- down; 4-- up; 8 -- contact */
    int id;   /*touch ID */
};



class chsc6x :public chsc6x_platform{
private:
    uint8_t real_suspend_flag = 1;
#if CHSC6X_GESTURE
    uint8_t g_gesture_enable = 1; 
    uint8_t g_gesture_status = 0;
#endif

public:
    chsc6x(TwoWire *wire = &Wire1, int sda_pin = -1, int scl_pin = -1, int int_pin = -1, int rst_pin = -1)
        : chsc6x_platform(wire, sda_pin, scl_pin, int_pin, rst_pin)
    {
    };
    ~chsc6x(){};
    void chsc6x_init(void);
    void chsc6x_suspend(void);
    void chsc6x_resume(void);
    int chsc6x_read_touch_info(uint16_t *touchX, uint16_t *touchY);

#if CHSC6X_GESTURE
    void chsc6x_gesture_check(void);
    uint8_t chsc6x_get_gesture(void);
#endif

};




#endif


