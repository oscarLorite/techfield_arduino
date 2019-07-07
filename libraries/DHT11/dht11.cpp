#include "dht11.h"

unsigned long time = 0;


int dht11::espera(uint8_t estat, uint8_t pin, unsigned long temps){
	while(digitalRead(pin) == estat){
		if((micros() - time) > temps) return 1;
	}
	return 0;
}


int dht11::read(uint8_t pin)
{
	uint8_t bits[5]; //Estrucutra per rebre dades
	uint8_t cnt = 7;
	uint8_t idx = 0;

	for (int i=0; i< 5; i++) bits[i] = 0;//Buidem el contingut del array bits


	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	delay(18);
	digitalWrite(pin, HIGH);
	delayMicroseconds(40);
	pinMode(pin, INPUT);
	
	time = micros();

	
	if(espera(LOW, pin, 100) != 0) return 1; //Si no es compleix l'espera => Timeout

	time=micros();
	if(espera(HIGH, pin, 100) != 0) return 1;
	time=micros();
	
	// Inici de la lectura de dades
	for (int i=0; i<40; i++)
	{
		time= micros();
		if(espera(LOW, pin, 70) != 0) return 1; 

		time = micros();

		if(espera(HIGH, pin, 100) != 0) return 1;

		if ((micros() - time) > 60) bits[idx] |= (1 << cnt);
		
		if (cnt == 0)   
		{
			cnt = 7;    
			idx++;      
		}
		else cnt--;
	}
	
	humidity    = bits[0]; 
	temperature = bits[2]; 

	uint8_t sum = bits[0] + bits[2];  

	if (bits[4] != sum){
		return 4;//Si no es compleix => Error en el checksum
	}else{	
		return 2; //Lectura correcta 
	}
}