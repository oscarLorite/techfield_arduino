#include "Photoresistor.h"

/** Constructor
*	Inicialitzacions dels pins
*
**/
Photoresistor::Photoresistor(uint8_t _pin)
{
	pin = _pin;
	pinMode(pin, INPUT);
}

/** funció readLight()
* 	Funció que fa la medicijó del LDR i obté el valor en Lux.
*	A més, fa el control d'errors per tal d'activar el watchDog.
*
*	Retorna:
*	0 => Lectura correcta.
*	-1 => Error en la lectura
*
**/
int Photoresistor::readLight(void)
{
	float _lux;
	float Vo;
	
	int _err = 0; //Variable control errror. 
	
	Vo = ((Vref/ADCres)*(analogRead(pin)));
	
	_lux = (((Vref*LuxRel)*Vo)-LuxRel)/R1;
	
	
	//Control d'error
	if(_lux < 0)
	{
		_lux = 0.0;
		_err = -1;
	}
	
	lux = _lux;
	
	return _err;
	
}
