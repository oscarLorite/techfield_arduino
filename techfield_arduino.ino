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

//Estructura de Dades

/** ESTRUCTURA DE DADES PER DESAR ELS VALORS DELS SENSORS
 * Permet desar els valors obtinguts dels sensors
 */
struct DadesSensades
{
  float lluminositat; //Valor lux sensor LDR
  int humitatTerreny; //Valor moisture sensor
  int temperaturaAmbient; //Valor temp. DHT11
  int humitatAmbient; //Valor humitat DHT11
  int data; //Cambiar a tipus date. 
};

/** ESTRUCUTRA DE DADES PER SABER L'ESTAT DEL PROGRAMA
 * Ens permet saber si ha hagut problemes en les diverses 
 * tasques que desenvolupa el programa.
 * 
 * En cas d'error, induirà a la màquina a fer un reinici. 
 * 
 * Variables guarden valors 0 ó -1:
 * 0 => Lectura correcta 
 * -1 => Error de lectura
 */
struct ControlErrors
{
  int errLluminositat; 
  int errHumitatTerreny; 
  int errTemperaturaAmbient; 
  int errHumitatAmbient; 
  int errData; 
};


//JSON inicialitzacions
const int BUFFER_SIZE = JSON_OBJECT_SIZE (5);
StaticJsonDocument<BUFFER_SIZE> doc;


//Inicialitzacions
DadesSensades dataValue;
ControlErrors error;

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
  //serialitzarJson();
  delay(1000);

}

void inicialitzacionsVariables(){
  dataValue.lluminositat = 0;
  dataValue.temperaturaAmbient = 0;
  dataValue.humitatAmbient = 0;
  dataValue.humitatTerreny = 0;
  dataValue.data = 0;

  error.errLluminositat = 0; 
  error.errHumitatTerreny = 0; 
  error.errTemperaturaAmbient = 0; 
  error.errHumitatAmbient = 0; 
  error.errData = 0; 
}

void obtenirLluminositat(){
  error.errLluminositat = ldr.readLight();
  dataValue.lluminositat = ldr.lux;
}

void obtenirDht11(){
  if (dht11.read(dhtPin) == 2){ //Lectura correcta
    dataValue.temperaturaAmbient = dht11.temperature;
    dataValue.humitatAmbient = dht11.humidity; 
  }else{ //Error checksum(2) o lectura incorrecta
    dataValue.temperaturaAmbient = -1000; //Dades Errònies
    dataValue.humitatAmbient = -1000;
  }
}

void obtenirHumitatTerreny(){
  moisture.readMappedValue();
  dataValue.humitatTerreny = moisture.moistureMappedValue;
}

void mesurarDades(){
  obtenirLluminositat();
  obtenirDht11();
  obtenirHumitatTerreny();
}

void escriureConsola(){
  Serial.println("-----DADES TECHFIELD----");
  Serial.print("Lluminositat: ");
  Serial.print(dataValue.lluminositat);
  Serial.println(" lux.");
  Serial.print("Detecció Error");
  Serial.println(error.errLluminositat);
  
  Serial.print("Temperatura ambient: ");
  Serial.print(dataValue.temperaturaAmbient);
  Serial.println(" ºC");
  
  Serial.print("Humitat ambiental: ");
  Serial.print(dataValue.humitatAmbient);
  Serial.println(" %");
  
  Serial.print("Humitat terreny: ");
  Serial.print(dataValue.humitatTerreny);
  Serial.println(" %");
  Serial.println("------------------------");
}

void serialitzarJson(){
  doc["sensorLDR"] = dataValue.lluminositat;
  doc["sensorDHT11temp"] = dataValue.temperaturaAmbient;
  doc["sensorDHT11humi"] = dataValue.humitatAmbient;
  doc["sensorMoisture"] = dataValue.humitatTerreny;
  doc["date"] = dataValue.data;
  serializeJsonPretty(doc,Serial);
}
