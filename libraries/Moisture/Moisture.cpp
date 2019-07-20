#include "Moisture.h"

/** Constructor
*	Inicialitzacions dels pins
*
**/
Moisture::Moisture(uint8_t _pin)
{
	pin = _pin;
	pinMode(pin, INPUT);
}

/** funci� readAnalogValue()
* 	Funci� que fa la medicij� del sensor Moisture i obt� el valor
*	A m�s, fa el control d'errors per tal d'activar el watchDog.
*
*	Retorna:
*	0 => Lectura correcta.
*	-1 => Error en la lectura
*
**/
int Moisture::readAnalogValue(void)
{
	int _moistureValue;
	int _err = 0;
	
	_moistureValue = analogRead(pin);
	
	//Control d'error
	if(_moistureValue < 310)
	{
		_moistureValue = 310;
		_err = -1;
	}
	
	moistureValue = _moistureValue;
	return _err;
}

/** funci� readAnalogValue()
* 	Funci� que fa la medicij� del sensor Moisture i obt� el valor
* 	en persentantge 
*	A m�s, fa el control d'errors per tal d'activar el watchDog.
*
*	Retorna:
*	0 => Lectura correcta.
*	-1 => Error en la lectura
*
**/
int Moisture::readMappedValue(void)
{
	float  _moistureMappedValue;
	int _moistureValue;
	int _err = 0;
	
	_moistureValue = analogRead(pin);
	
	//Error Control 
	if(_moistureValue < 320)
	{
		_moistureValue = 320; //Completly wet
		_err = -1;
	}
	if(_moistureValue > 630)
	{
		_moistureValue = 630; //Completly dry
		_err = -1; //Completly dry
	} 
	
	/* Mapping the value */
	_moistureMappedValue = map(_moistureValue,MinCapacitiveValue,MaxCapacitiveValue, 100, 0);
	
	moistureMappedValue = _moistureMappedValue;
	
	return _err;
}
