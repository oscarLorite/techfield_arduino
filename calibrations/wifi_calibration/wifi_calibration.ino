/*  ----------------------------------------------------------------
   WIFI AMB ESP8266

   Programa que configura el component ESP8266:
    - Connexió a xarxa wifi
    - Enviament de peticions GET a un port i direcció IP concretes.
   
------------------------------------------------------------------  */


/* DADES INICIALITZACIÓ ESP8266
 *  Informació relativa a la xarxa WiFi - Nom i clau
 *  Informació relativa a la IP enviament dades i port
*/
String wifiNetwork = "LJHome_2G"; //Nom xarxa WiFi. 
String wifiPass = "bueeQsR3"; //Contrassenya wifi

String destinationIP = "184.106.153.149"; //IP on volem enviar la petició GET
String destinationPort = "80"; //Num. del port.

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

String cadena="";
int variable=10000;

void setup(){ 
 /* Serial1.begin(115200);
  Serial.begin(115200);
  
  Serial1.setTimeout(2000);
  
  setUpWifi(); */
}

int i=0;
void loop()
   { 
    /*SendData();
    while(i>5){
      resetWifiModule();
      i=0;
    }
    i++;

    
  delay(10000); //pausa de 10seg*/
   }

void setUpWifi(){
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
    Serial.println("Error al ESP8266");
  }
}

void resetWifiModule(){
  //-----Resetear módulo ESP8266-------//Podemos comentar si el ESP ya está configurado
    Serial1.println("AT+RST");
    if(Serial1.find("OK")){
      Serial.println("ESP8266 reseteado");
      //setUpWifi();
      return 0;
    }
      
  
}

void SendData(){
  Serial1.println(ordresSendData[0]);
    if( Serial1.find("OK"))
      {  
          Serial.println();
          Serial.println();
          Serial.println("ESP8266 connectant amb el servidor...");             
    
          //Comencem a montar l'encapçalament de la petició HTTP
          String peticioHTTP= "GET /update?key=W4Z9ZH1OF0F58I2R&field2="+String(variable)+"\r\n";
          //peticionHTTP=peticionHTTP+String(variable1)+"&b="+String(variable2)+" HTTP/1.1\r\n";
          //peticionHTTP=peticionHTTP+"Host: www.aprende-web.net\r\n\r\n";
    
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
                    variable=variable+100;
                  }
              }   
            }
            else
            {
              Serial.println("No s'ha pogut enviar la petició...");
           }            
          }
      }
      else
      {
        Serial.println("No s'ha pogut connectar amb el servidor");
      }
}
