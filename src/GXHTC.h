#ifndef __GXHTC_H__
#define __GXHTC_H__
#include <Arduino.h>


#define GXHTC_SDA          (1)
#define GXHTC_SCL          (2)
#define GXHTC_ADDRESS      (0x70)
#define GXHTC_AVERAGE_NUM  (5)
#define GXHTC_CHIP_ID      (0x0887)

class GXHTC {
public:
	GXHTC();
	void begin(int sda =GXHTC_SDA,int scl =GXHTC_SCL ,uint8_t address=GXHTC_ADDRESS);
	void end();
  	float g_temperature=0;
	float g_humidity=0;
	void read_data();
	uint16_t read_id(void);

private:
	uint8_t _address;
	float  findAverage(float arr[], int size);
};

#endif

