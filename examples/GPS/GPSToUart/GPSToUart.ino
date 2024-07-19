#include "Arduino.h"
#include "HT_TinyGPS++.h"


TinyGPSPlus GPS;

#define VGNSS_CTRL 3


void GPS_test(void)
{
	pinMode(VGNSS_CTRL,OUTPUT);
	digitalWrite(VGNSS_CTRL,HIGH);
	Serial1.begin(115200,SERIAL_8N1,33,34);    
	Serial.println("GPS_test");

	delay(100);

	while(1)
	{
		if(Serial1.available()>0)
		{
			if(Serial1.peek()!='\n')
			{
				GPS.encode(Serial1.read());
			}
			else
			{
				Serial1.read();
				if(GPS.time.second()==0)
				{
					continue;
				}
				Serial.printf(" %02d:%02d:%02d.%02d",GPS.time.hour(),GPS.time.minute(),GPS.time.second(),GPS.time.centisecond());
        Serial.print("LAT: ");
        Serial.print(GPS.location.lat(), 6);
        Serial.print(", LON: ");
        Serial.print(GPS.location.lng(), 6);
        Serial.println();
       
				delay(5000);
				while(Serial1.read()>0);
			}
		}
	}
}

void setup(){
  Serial.begin(115200);    
  Serial.println("GPS_test");
  delay(1000);
  GPS_test();
}

void loop(){
    delay(100);
}
