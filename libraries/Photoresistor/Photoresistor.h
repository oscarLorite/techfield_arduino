#include "Arduino.h"

#define PHOTO_LIB_VERSION 0.1

/* Class declaration */
class Photoresistor
{
	public:
		Photoresistor(uint8_t _pin);
		float lux;
		void readLight(void);
	protected:
		uint8_t pin;
		const  float ADCres = 1023.0;
		const  float R1 = 10.0;
		const  float Vref = 5.0;
		const  float LuxRel = 500.0;
	private:
};
