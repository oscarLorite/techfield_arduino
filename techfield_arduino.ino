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

int periodeLectura = 1000; //Període entre lectures.
int repBucle = 300;
//Delay 1000*300 = 300000 -> 5min

//Contador d'errors
int cntErrSensor = 0;
int cntErrWifi = 0;
int reiniciatESP8266 = 0;

//Variables que desen els codis d'errors 
int codiErrSensor = 0; //Codi error sensors
int codiErrWifi = 0; //Codi error ESP8266

//Errors de lectura de sensors permesos abans del reinici
int numErrLecturesPermesos = 10;
int numErrWifiPermesos = 10;


//Estructura de Dades

/** ESTRUCTURA DE DADES PER DESAR ELS VALORS DELS SENSORS
 * Permet desar els valors obtinguts dels sensors
 */
struct DadesSensades
{
  int const idTechfield = 1; //Número identificatiu de la placa. 
  float lluminositat; //Valor lux sensor LDR
  int humitatTerreny; //Valor moisture sensor
  int temperaturaAmbient; //Valor temp. DHT11
  int humitatAmbient; //Valor humitat DHT11 
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
};

/* DADES INICIALITZACIÓ ESP8266
 *  Informació relativa a la xarxa WiFi - Nom i clau
 *  Informació relativa a la IP enviament dades i port
 *  
 *  String cadena -> Desa la resposta que rebem un cop enviada la petició 
*/
String wifiNetwork = "LJHome_2G"; //Nom xarxa WiFi. 
String wifiPass = "bueeQsR3"; //Contrassenya wifi

String destinationIP = "184.106.153.149"; //IP on volem enviar la petició GET
String destinationPort = "80"; //Num. del port.

String cadena="";

/*SEQÜÈNCIA DE COMANDES AT - INICIAR ESP2866*/
String ordreConfig[] = {
  "AT+CWMODE=1",
  "AT+CWJAP=\""+wifiNetwork+"\",\""+wifiPass+"\"",
  "AT+CIPMUX=0"
};

/*SECUENCIA DE COMANDES  AT - ENVIAR DADES A TRAVÉS ESP2866*/
String ordresSendData[] = {
  "AT+CIPSTART=\"TCP\",\""+destinationIP+"\","+destinationPort+"",
  "AT+CIPSEND=",
  "AT+CIPCLOSE"
};


//JSON inicialitzacions
const int BUFFER_SIZE = JSON_OBJECT_SIZE (4);
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

  error.errLluminositat = 0; 
  error.errHumitatTerreny = 0; 
  error.errDht11 = 0;

  Serial.println("Variables inicialitzades");
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
 * Retorna:
 * 0 -> Lectura correcta.
 * 9 -> Error ldr
 * 90 -> Error dht11
 * 900 -> Error soil moisture sensor
 * 
 * Nota: Si hi ha més d'un error, retorna la suma dels errors.
 * Ex.: Error lectura ldr + lectura dht11 -> Retorna: 99.
 */
int comprovarDades(){
  int estatLectura = 0;
  
  if (error.errLluminositat != 0)
  {
    estatLectura = estatLectura+9;
  }
  if (error.errHumitatTerreny != 0)
  {
    estatLectura = estatLectura+900;
  }
  if (error.errDht11 != 0)
  {
    estatLectura = estatLectura+90;
  }
  
  return estatLectura;  
}
 

/** FUNCIÓ WATCHDOG.
 * Controla el número d'errors de lectura de les dades.
 * Controla el número d'errors d'enviaments de paquets.
 * 
 * Comptabilitza el número d'errors.
 * Als 10 errors de lectura de les dades fa un reset del programa.
 * Als 10 errors d'enviament de les dades fa un reset del modul Wifi
 * Si hi ha un Error al Wifi un cop s'ha resetejat el mòdul -> Reset Arduino.
 */
void watchDog(){
  //Actualitzem contadors d'errors de lecutres de sensors i del wifi. 
  if(codiErrSensor != 0)
  {
    cntErrSensor++;
  }
  if(codiErrWifi != 0)
  {
    cntErrWifi++;
  }

  /* ---- CONTROL D'ERRORS ----- */

  /* RESETEJAR ARDUINO SI: 
   *  - hi ha 10 o més lectures de sensors errònies
   *  - s'ha reiniciat un cop el mòdul wifi.
   */
  if(cntErrSensor >= numErrLecturesPermesos || reiniciatESP8266 >1 )
  {
    Serial.println("Procedint a resetejar....");
    delay(100);
    resetFunc(); //Resetejar Arduino
    Serial.println("Resetejat! :) ");
  }

  //Resetejem NOMÉS el módul Wifi i el tornem a configurar.
  if(cntErrWifi >= numErrWifiPermesos)
  {
    resetWifiModule(); //Resetejar modul Wifi
    setUpWifi();//SetUP Wifi
    reiniciatESP8266++;
    cntErrWifi=0;
  }
  
}

void escriureConsola(){
  Serial.println();
  Serial.println();
  Serial.print("----DADES TECHFIELD id: ");
  Serial.print(dataValue.idTechfield);
  Serial.println(" ----");

  Serial.print("Lectures cada: ");
  Serial.print((periodeLectura*repBucle));
  Serial.println(" milisegons.");

  Serial.println("*** VALORS SENSORS ***");
  Serial.print("\t - Lluminositat: ");
  Serial.print(dataValue.lluminositat);
  Serial.println(" lux.");

  Serial.print("\t - Temperatura ambient: ");
  Serial.print(dataValue.temperaturaAmbient);
  Serial.println(" ºC");

  Serial.print("\t - Humitat Ambiental: ");
  Serial.print(dataValue.humitatAmbient);
  Serial.println(" %");

  Serial.print("\t - Humitat Terreny: ");
  Serial.print(dataValue.humitatTerreny);
  Serial.println(" %");
  
  Serial.println("*** ERRORS LECTURA ***");
  Serial.print("\t Codi error lectura sensors: ");
  Serial.println(codiErrSensor);

  Serial.println("*** COMPTABILITZACIÓ D'ERRORS ***");
  Serial.print("\t - Lectures sensors: ");
  Serial.println(cntErrSensor);
  Serial.print("\t - Mòdul WiFi: ");
  Serial.println(cntErrWifi);
  Serial.print("\t - Reset mòdul WiFi?  ");
  Serial.println(reiniciatESP8266);
  Serial.println("----------------------------");
}


void serialitzarJson(){
  doc["sensorLDR"] = dataValue.lluminositat;
  doc["sensorDHT11temp"] = dataValue.temperaturaAmbient;
  doc["sensorDHT11humi"] = dataValue.humitatAmbient;
  doc["sensorMoisture"] = dataValue.humitatTerreny;
  serializeJsonPretty(doc,Serial);
}

/* FUNCIÓ QUE INICIALITZA EL ESP8266 
 * 
 * Es connecta a la xarxa wifi concreta a través de comandes AT
 * 
 * Retorna:
 * 0 => Connexió correcta a la xarxa wifi.
 * -1 => Error en la connexió a la xarxa wifi. 
 */
int setUpWifi(){
  int estat = 0;
  //Verifiquem que ESP8266 respon a les comandes AT
  Serial1.println("AT");
  if(Serial1.find("OK"))
  {
    Serial.println("Resposta AT correcte");

    /*-----Configuració de la xarxa-------*/
    //ESP8266 en mode estació (connexió a una xarxa WiFi )
    Serial1.println(ordreConfig[0]);
    if(Serial1.find("OK"))
    {
      Serial.println("ESP8266 en mode '1' > Mode estació");
    }
    
    //Connexió a una xarxa WiFi
    Serial1.println(ordreConfig[1]);
    Serial.println("Connectat a la xarxa ...");
    Serial1.setTimeout(10000); //Augmentem timeout si hi ha retard a la connexió
    if(Serial1.find("OK"))
    {
      Serial.println("WIFI connectat");
    }
    else
    {
      Serial.println("Error al conectar-se a la xarxa ");
    }
    Serial1.setTimeout(2000);
    //Deshabilitem les múltiples connexions 
    Serial1.println(ordreConfig[2]);
    if(Serial1.find("OK"))
    {
      Serial.println("Multiconnexions deshabilitades");
    }
    /*-----FI configuració de la xarxa-------*/
    delay(1000);  
    
  }
  else
  {
    estat = -1;
  }
  return estat;
}


/* FUNCIÓ QUE REINICIA  EL ESP8266 
 * Fa un reset del mòdul a través de comandes AT
 */
void resetWifiModule(){
  //-----Resetear módulo ESP8266-------//Podemos comentar si el ESP ya está configurado
    Serial1.println("AT+RST");
    if(Serial1.find("OK")){
      Serial.println("ESP8266 reseteado");
    }
}



/* FUNCIÓ QUE PERMET ENVIAR DADES AL SERVIDOR A TRAVÉS DEL MÒDUL ESP8266 
 * 
 * Crea un paquet TCP amb destí la IP i el port proporcionat.
 * Omple el paquet amb la petició GET i els paràmetres a enviar.
 * Envia el paquet paquet a la direcció establerta
 * 
 * Paràmetres:
 *  techId: identificador del techField.
 *  ldr: valor en lux de la intensitat lluminosa. (float)
 *  dhtT: temperatura ambiental (int)
 *  dhtH: humitat ambiental (%) (int)
 *  soilM: humitat del terreny (%) (int)
 *  err: codi d'error.
 * 
 * Retorna:
 * 0 => Petició amb les dades enviades correctament al servidor.
 * -1 => Error en l'enviament de dades. 
 * -2 => No s'ha pogut connectar amb el servidor.
 */
int SendData(int techId, float ldr, int dhtT, int dhtH, int soilM, int err){
  int estat = 0;
  
  Serial1.println(ordresSendData[0]);
    if( Serial1.find("OK"))
      {  
          Serial.println();
          Serial.println();
          Serial.println("ESP8266 connectant amb el servidor...");             
    
          //Comencem a montar l'encapçalament de la petició HTTP
          String peticioHTTP= "GET /update?key=W4Z9ZH1OF0F58I2R&field1="+String(techId)+"&field2="+String(ldr);
          peticioHTTP=peticioHTTP+"&field3="+String(dhtT)+"&field4="+String(dhtH)+"&field5="+String(soilM)+"&field6="+String(err)+"\r\n HTTP/1.1\r\n";
          peticioHTTP=peticioHTTP+"Host: www.techfield.com \r\n\r\n";
    
          //Enviem la mida en caracters de la petició  
          Serial1.print(ordresSendData[1]);
          Serial1.println(peticioHTTP.length());

          //esperem a ">" para començar a enviar la petició
          if(Serial1.find(">")) // ">" podem començar a enviar la petició
          {
            Serial.println("Enviant dades...");
            Serial1.println(peticioHTTP);
            if( Serial1.find("SEND OK"))
            {  
              Serial.println("Dades enviades:");
              Serial.println();
              Serial.println(peticioHTTP);
              Serial.println("Esperant resposta...");
              
              boolean fi_resposta=false; 
              long tiempo_inicio=millis(); 
              cadena="";
              
              while(fi_resposta==false)
              {
                  while(Serial1.available()>0) 
                  {
                      char c=Serial1.read();
                      Serial.write(c);
                      cadena.concat(c);  //guardamos la respuesta en el string "cadena"
                  }
                  //finalizamos si la respuesta es mayor a 500 caracteres
                  if(cadena.length()>500) //Pueden aumentar si tenen suficiente espacio en la memoria
                  {
                    Serial.println("La respuesta es massa gran");
                    
                    Serial1.println(ordresSendData[2]);
                    if( Serial1.find("OK"))
                      Serial.println("Connexió finalitzada");
                    fi_resposta=true;
                  }
                  if((millis()-tiempo_inicio)>10000) //Si passen 10segons finalitzem la connexió
                  {
                    Serial.println("Temps d'espera esgotat");
                    Serial1.println(ordresSendData[2]);
                    if( Serial1.find("OK"))
                      Serial.println("Connexió finalitzada");
                    fi_resposta=true;
                  }
                  if(cadena.indexOf("CLOSED")>0) //si recibimos un CLOSED significa que ha finalizado la respuesta
                  {
                    Serial.println();
                    Serial.println("Cadena rebuda correctament, connexió finalitzada");         
                    fi_resposta=true;
                  }
              }   
            }
            else
            {
              estat=-1;
              Serial.println("No s'ha pogut enviar la petició...");
           }            
          }
      }
      else
      {
        estat=-2;
        Serial.println("No s'ha pogut connectar amb el servidor");
      }
 return estat;
}





void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  Serial1.setTimeout(2000);
  
  inicialitzacionsVariables();
  if(setUpWifi() == 0)
  {
    Serial.println("Connexió correcta a la xarxa WIFI");
  }
  else
  {
    reiniciatESP8266++; //Sumem l'error al comptador de reinici del ESP8266
    Serial.println("Error al ESP8266");
  }
}

void loop() {
  mesurarDades();
  codiErrSensor = comprovarDades();

  //Enviar dates
  codiErrWifi = SendData(dataValue.idTechfield, dataValue.lluminositat, dataValue.temperaturaAmbient, dataValue.humitatAmbient, dataValue.humitatTerreny, codiErrSensor);
  
  //Executem WatchDog per veure l'estat del sistema. 
  watchDog();

  //Escriure per consola.
  escriureConsola();

  for (int x = 0; x < repBucle; x++)
  {
    delay(periodeLectura);
  }
}
