//Llibreries
#include "Photoresistor.h"
#include "dht11.h"
#include "Moisture.h"
#include <ArduinoJson.h>


//Variables globals
const int dhtPin = A1;
const int ldrPin = A0;
const int moisturePin = A2;
const int test = 0;

float lluminositat, humitatTerreny;
int temperaturaAmbient, humitatAmbient;


//JSON inicialitzacions
const int BUFFER_SIZE = JSON_OBJECT_SIZE (5);
StaticJsonDocument<BUFFER_SIZE> doc;


//Inicialitzacions
dht11 dht11;
Photoresistor ldr(ldrPin);
Moisture moisture(moisturePin);

void setup() {
  Serial.begin(9600);
  inicialitzacionsVariables();
  
}

void loop() {
  mesurarDades();
  escriureConsola();
  serialitzarJson();
  delay(1000);

}

void inicialitzacionsVariables(){
  lluminositat = 0;
  temperaturaAmbient = 0;
  humitatAmbient = 0;
  humitatTerreny = 0.0;
}

void obtenirLluminositat(){
  ldr.readLight();
  lluminositat = ldr.lux;
}

void obtenirDht11(){
  if (dht11.read(dhtPin) == 2){ //Lectura correcta
    temperaturaAmbient = dht11.temperature;
    humitatAmbient = dht11.humidity; 
  }else{ //Error checksum(2) o lectura incorrecta
    temperaturaAmbient = -1000; //Dades Errònies
    humitatAmbient = -1000;
  }
}

void obtenirHumitatTerreny(){
  moisture.readMappedValue();
  humitatTerreny = moisture.moistureMappedValue;
}

void mesurarDades(){
  obtenirLluminositat();
  obtenirDht11();
  obtenirHumitatTerreny();
}

void escriureConsola(){
  Serial.println("-----DADES TECHFIELD----");
  Serial.print("Lluminositat: ");
  Serial.print(lluminositat);
  Serial.println(" lux.");
  Serial.print("Temperatura ambient: ");
  Serial.print(temperaturaAmbient);
  Serial.println(" ºC");
  Serial.print("Humitat ambiental: ");
  Serial.print(humitatAmbient);
  Serial.println(" %");
  Serial.print("Humitat terreny: ");
  Serial.print(humitatTerreny);
  Serial.println(" %");
  Serial.println("------------------------");
}

void serialitzarJson(){
  doc["sensorLDR"] = lluminositat;
  doc["sensorDHT11temp"] = temperaturaAmbient;
  doc["sensorDHT11humi"] = humitatAmbient;
  doc["sensorMoisture"] = humitatTerreny;
  serializeJsonPretty(doc,Serial);
}
