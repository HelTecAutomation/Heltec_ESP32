#include <Wire.h>
#include "GXHTC.h"


GXHTC::GXHTC()
{
}

void GXHTC::begin(int sda,int scl,uint8_t address) 
{
	_address = address;
	Wire.begin(sda,scl);
}

void GXHTC::end() 
{
	Wire.end();
}

uint16_t GXHTC::read_id(void)
{
  	uint8_t buff[2];
	uint16_t chip_id;
	Wire.beginTransmission(_address);
	Wire.write(0xEF);
	Wire.write(0xC8);
	Wire.endTransmission();
	
	Wire.requestFrom(_address, (uint8_t)2);
	buff[0] = Wire.read();
	buff[1] = Wire.read();
	
	uint16_t tem,hum;
	
  	chip_id = ((buff[0]<<8) | buff[1]);
	return chip_id;
}
void GXHTC::read_data()
{
  	uint8_t buff[6];
	Wire.beginTransmission(_address);
	Wire.write(0x35);
	Wire.write(0x17);
	Wire.endTransmission();
	
	Wire.beginTransmission(_address);
	Wire.write(0x7C);
	Wire.write(0xA2);
	Wire.endTransmission();
	
	Wire.requestFrom(_address, (uint8_t)6);
	buff[0] = Wire.read();
	buff[1] = Wire.read();
	buff[2] = Wire.read();
	buff[3] = Wire.read();
	buff[4] = Wire.read();
	buff[5] = Wire.read();
	
	uint16_t tem,hum;
	
  	tem = ((buff[0]<<8) | buff[1]);//�¶�ƴ��
	hum = ((buff[3]<<8) | buff[4]);//ʪ��ƴ��
	
	g_temperature= (175.0*(float)tem/65535.0-45.0) ;// T = -45 + 175 * tem / (2^16-1)
	g_humidity= (100.0*(float)hum/65535.0);// RH = hum*100 / (2^16-1)
	
	Wire.beginTransmission(_address);
	Wire.write(0xb0);
	Wire.write(0x98);
	Wire.endTransmission();
}

float  GXHTC::findAverage(float arr[], int size) 
{
	float max = arr[0];  
	float min = arr[0];  
	float sum = 0;
	int count = 0;
	for (int i = 0; i < size; i++) {
		if (arr[i] > max)
			max = arr[i];
		if (arr[i] < min)
			min = arr[i];
		sum += arr[i];
		count++;
	}
	count -= 2;
	sum = sum - max - min;
	float average = (float)sum / count;

	return average;
}
