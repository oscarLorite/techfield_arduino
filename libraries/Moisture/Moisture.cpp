#include "Moisture.h"

Moisture::Moisture(uint8_t _pin)
{
	pin = _pin;
	pinMode(pin, INPUT);
}

void Moisture::readAnalogValue(void)
{
	int  _moistureValue;
	
	_moistureValue = analogRead(pin);
	
	/* Error Control */
	if(_moistureValue < 310) _moistureValue = 310;
	
	moistureValue = _moistureValue;
	
}

void Moisture::readMappedValue(void)
{
	float  _moistureMappedValue;
	int _moistureValue;
	
	_moistureValue = analogRead(pin);
	
	/* Error Control */
	if(_moistureValue < 320) _moistureValue = 320; //Completly wet
	if(_moistureValue > 630) _moistureValue = 630; //Completly dry
	
	
	/* Mapping the value */
	
	_moistureMappedValue = map(_moistureValue,MinCapacitiveValue,MaxCapacitiveValue, 100, 0);
	
	moistureMappedValue = _moistureMappedValue;
	
}
