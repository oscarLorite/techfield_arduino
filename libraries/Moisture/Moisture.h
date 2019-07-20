#include "Arduino.h"

#define MOISTURE_LIB_VERSION 0.1

/* Class declaration */
class Moisture
{
	public:
		Moisture(uint8_t _pin);
		int moistureValue;
		float moistureMappedValue;
		int readAnalogValue(void);
		int readMappedValue(void);
	protected:
		uint8_t pin;
		const  int MaxCapacitiveValue = 630; //Completly dry sensor
		const  int MinCapacitiveValue = 320; //Completly wet sensor
	private:
};
