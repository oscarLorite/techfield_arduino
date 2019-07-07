#include <Arduino.h>


class dht11
{
public:
    int read(uint8_t pin);
	int humidity;
	int temperature;
private:
	int espera(uint8_t estat, uint8_t pin, unsigned long temps);
};

