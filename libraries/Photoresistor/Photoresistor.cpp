#include "Photoresistor.h"

Photoresistor::Photoresistor(uint8_t _pin)
{
	pin = _pin;
	pinMode(pin, INPUT);
}

void Photoresistor::readLight(void)
{
	float _lux;
	float Vo;
	
	Vo = ((Vref/ADCres)*(analogRead(pin)));
	
	_lux = (((Vref*LuxRel)*Vo)-LuxRel)/R1;
	
	/*Error Control*/
	if(_lux < 0) _lux = 0.0;
	
	lux = _lux;
	
}
