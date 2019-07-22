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

//Contador d'errors
int cntErr = 0;

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
 * 
 */
struct ControlErrors
{
  int errLluminositat; 
  int errHumitatTerreny; 
  int errDht11;
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


/* FUNCIÓ RESET 
 * Funció que permetrà ressetejar la placa Arduino.
 * Copiarà un 0 a la posició de memòria @ 0X0000.
 */
void(* resetFunc) (void) = 0;

void inicialitzacionsVariables(){
  dataValue.lluminositat = 0;
  dataValue.temperaturaAmbient = 0;
  dataValue.humitatAmbient = 0;
  dataValue.humitatTerreny = 0;
  dataValue.data = 0;

  error.errLluminositat = 0; 
  error.errHumitatTerreny = 0; 
  error.errDht11 = 0;
  error.errData = 0; 
}

void obtenirLluminositat(){
  error.errLluminositat = ldr.readLight();
  
  dataValue.lluminositat = ldr.lux; 
}

void obtenirDht11(){
  error.errDht11 = dht11.read(dhtPin);
  
  dataValue.temperaturaAmbient = dht11.temperature;
  dataValue.humitatAmbient = dht11.humidity;
}

void obtenirHumitatTerreny(){
  error.errHumitatTerreny = moisture.readMappedValue();
   
  dataValue.humitatTerreny = moisture.moistureMappedValue;
}

void mesurarDades(){
  obtenirLluminositat();
  obtenirDht11();
  obtenirHumitatTerreny();
}

/* FUNCIÓ QUE COMPROVA LA CORRECTA LECTURA DE LES DADES
 * 
 * 
 * Retorna:
 * 0 => Lectura correcta.
 * -1 => Alguna de les lectures és incorrecta. 
 */
int comprovarDades(){
  int estatLectura = 0;

  if (error.errLluminositat != 0 || error.errHumitatTerreny != 0 || error.errDht11 != 0)
  {
    estatLectura = -1;
  }

  return estatLectura;  
}
 

/** FUNCIÓ WATCHDOG.
 * Controla el número d'errors de lectura.
 * Si hi ha errors no envia les dades.
 * Si NO hi ha errors envia les dades.
 * 
 * Comptabilitza el número d'errors.
 * Als 10 errors de lectura fa un reset del programa.
 */
void watchDog(){
  if (comprovarDades() == 0) //Totes les lectures correctes
  {
    escriureConsola();
    //serialitzarJson();
  }else
  {
    escriureConsolaError();
  
    cntErr++; //Incrementar cnt error.

    if(cntErr >= 10)
    {
      Serial.println("Procedint a resetejar....");
      delay(100);
      resetFunc();
      Serial.println("Resetejat!");
    }
  }
}

void escriureConsola(){
  Serial.println("-----DADES TECHFIELD----");
  Serial.print("Lluminositat: ");
  Serial.print(dataValue.lluminositat);
  Serial.println(" lux.");
  Serial.print("Detecció Error: ");
  Serial.println(error.errLluminositat);
  
  Serial.print("Temperatura ambient: ");
  Serial.print(dataValue.temperaturaAmbient);
  Serial.println(" ºC");
  
  Serial.print("Humitat ambiental: ");
  Serial.print(dataValue.humitatAmbient);
  Serial.println(" %");
   Serial.print("Detecció Error DHT11: ");
  Serial.println(error.errDht11);
  
  Serial.print("Humitat terreny: ");
  Serial.print(dataValue.humitatTerreny);
  Serial.println(" %");
  Serial.print("Detecció Error: ");
  Serial.println(error.errHumitatTerreny);
  Serial.println("------------------------");
}

void escriureConsolaError(){
  Serial.println("-----ERROR: DADES TECHFIELD----");
  Serial.println("Hi ha un error en les lectures.");
  Serial.print("Num. d'errors de lectura comptabilitzats: ");
  Serial.println(cntErr);
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

void setup() {
  Serial.begin(9600);
  inicialitzacionsVariables();
}
  

void loop() {
  mesurarDades();
  watchDog();
  delay(1000);
}
