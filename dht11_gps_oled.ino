#include <SoftwareSerial.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//librerias del ESP WIFI
#include <ESP8266WiFi.h>

//pantalla y ESP8266 - wifi
#include <SPI.h>
#include <Wire.h>

//libreria del gps
#include <TinyGPS.h>

// Incluimos librería sesnor
#include <DHT.h>

//pantalla
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
TinyGPS gps;
SoftwareSerial softSerial(D4, D3); //(TX, RX)

// Definimos el pin digital donde se conecta el sensor
#define DHTPIN D2
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

//pines de la pantalla
#define OLED_MOSI 14
#define OLED_CLK 16
#define OLED_DC 13
#define OLED_CS 15
#define OLED_RESET 12


//IP or name of address root: ie: google.com
//NOT google.com/nothing/after/the/dotcom.html
const char* hostGet = "danielvillegas.cloudaccess.host";

const char* ssid = "CATDAN";
const char* password = "8mp3400@";

//sensor de audio
const int sampleWindow = 250; // Sample window width in mS
unsigned int sample;

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#if (SSD1306_LCDHEIGHT != 64)

#endif

int WiFiCon() {
    // Check if we have a WiFi connection, if we don't, connect.
    int xCnt = 0;

    if (WiFi.status() != WL_CONNECTED) {

        Serial.println();
        Serial.println();
        Serial.print("Conectado a: ");
        Serial.println(ssid);

        display.begin(SSD1306_SWITCHCAPVCC); // Inicia el display OLED (Carga Buffer)
        //latitud
        display.clearDisplay(); // Borrar imagen en el OLED
        display.setTextSize(0.5); // Definir Tamaño del Texto
        display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
        display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
        display.println("Conectado a: "); // Carga la información al buffer
        display.println(ssid); // Carga la información al buffer
        display.display(); // Actualiza display con datos en Buffer
        delay(2000); // Demora de 2 segundos.

        WiFi.mode(WIFI_STA);

        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED && xCnt < 50) {
            delay(500);
            Serial.print(".");
              //Cuando se quiera mostrar nueva información
              display.clearDisplay(); // Borrar imagen en el OLED
              display.setTextSize(0.5); // Definir Tamaño del Texto
              display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
              display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
              display.println("cargando..."); // Carga la información al buffer
              display.display(); // Actualiza display con datos en Buffer
              delay(2000); // Demora de 2 segundos.
            xCnt++;
        }

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFiCon=0");
            return 0; //never connected
        } else {
            Serial.println("WiFiCon=1");
            Serial.println("");
            Serial.println("WiFi conectado");
            Serial.println("Direccion IP: ");
            Serial.println(WiFi.localIP());
            
            //wifi conectado
            display.clearDisplay(); // Borrar imagen en el OLED
            display.setTextSize(0.5); // Definir Tamaño del Texto
            display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
            display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
            display.println("Wifi conectado: "); // Carga la información al buffer
            display.println(WiFi.localIP()); // Carga la información al buffer
            display.display(); // Actualiza display con datos en Buffer
            delay(2000); // Demora de 2 segundos.
              
            return 1; //1 is initial connection
        }

    } else {
        Serial.println("WiFiCon=2");
        return 2; //2 is already connected

    }
}

void setup(){
   Serial.begin(115200);
   softSerial.begin(9600);

   // Comenzamos el sensor DHT
   dht.begin();

   //wifi
   WiFiCon();
}
 
void loop(){
   bool newData = false;
   unsigned long chars;
   unsigned short sentences, failed;
   
   // Intentar recibir secuencia durante un segundo
   for (unsigned long start = millis(); millis() - start < 1000;){
      while (softSerial.available()){
         char c = softSerial.read();
         if (gps.encode(c)) // Nueva secuencia recibida
            newData = true;
      }
   }

   if (WiFiCon() > 0){

            if(newData){
              
                float flat, flon;
                unsigned long age;
                gps.f_get_position(&flat, &flon, &age);
                Serial.print("LAT=");
                Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
                Serial.print(" LON=");
                Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);

                WiFiClient clientGet;
                const int httpGetPort = 80;
    
                //the path and file to send the data to:
                String urlGet = "/datagps.php";

                /*
                 * sensor de temperatura y humedad
                */
                // Leemos la humedad relativa
                float h = dht.readHumidity();
                // Leemos la temperatura en grados centígrados (por defecto)
                float t = dht.readTemperature();
                // Leemos la temperatura en grados Fahreheit
                float f = dht.readTemperature(true);
               
                // Comprobamos si ha habido algún error en la lectura
                if (isnan(h) || isnan(t) || isnan(f)) {
                  Serial.println("Error obteniendo los datos del sensor DHT11");
                  return;
                }

                // Calcular el índice de calor en Fahreheit
                float hif = dht.computeHeatIndex(f, h);
                // Calcular el índice de calor en grados centígrados
                float hic = dht.computeHeatIndex(t, h, false);
                
                Serial.print("Humedad: ");
                Serial.print(h);
                
                Serial.print("Temperatura: ");
                Serial.print(t);
         

                /*sensor de humedad y temperatura*/
    
                // We now create and add parameters:
                String temperatura = String(t);
                String humedad = String(h);
                float latitud = flat;
                float longitud = flon;

                urlGet += "?humedad=" + humedad + "&temperatura=" + temperatura + "&lat=" + latitud + "&lon=" + longitud;
                
                Serial.print(">>> Connecting to host: ");
                Serial.println(hostGet);

                //serviidor
                display.clearDisplay(); // Borrar imagen en el OLED
                display.setTextSize(0.5); // Definir Tamaño del Texto
                display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
                display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
                display.println("Enviado al servidor"); // Carga la información al buffer
                display.println("cloudaccess.host"); // Carga la información al buffer
                display.display(); // Actualiza display con datos en Buffer
                delay(1000); // Demora de 2 segundos.

                  if(!clientGet.connect(hostGet, httpGetPort)){
                      Serial.print("Connection failed: ");
                      Serial.print(hostGet);
                  }else{
                      clientGet.println("GET " + urlGet + " HTTP/1.1");
                      clientGet.print("Host: ");
                      clientGet.println(hostGet);
                      clientGet.println("User-Agent: ESP8266/1.0");
                      clientGet.println("Connection: close\r\n\r\n");
      
                      unsigned long timeoutP = millis();
                      while (clientGet.available() == 0) {
                          if (millis() - timeoutP > 10000) {
                              Serial.print(">>> Client Timeout: ");
                              Serial.println(hostGet);
                              clientGet.stop();
                              return;
                          }
                      }
      
                      //just checks the 1st line of the server response. Could be expanded if needed.
                      while (clientGet.available()){
                          String retLine = clientGet.readStringUntil('\r');
                          Serial.println(retLine);
                          break;
                      }
      
                  }//end client connection
      
                  Serial.print(">>> Closing host: ");
                  Serial.println(hostGet);
      
                  clientGet.stop();
                  delay(1000);
            }
            
   }
 
   if (newData){
      float flat, flon;
      unsigned long age;
      gps.f_get_position(&flat, &flon, &age);
      Serial.print("LAT=");
      Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
      Serial.print(" LON=");
      Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
      Serial.print(" SAT=");
      Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
      Serial.print(" PREC=");
      Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());

        //latitud
        display.clearDisplay(); // Borrar imagen en el OLED
        display.setTextSize(0.5); // Definir Tamaño del Texto
        display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
        display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
        display.println("Lat: "); // Carga la información al buffer
        display.println(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6); // Carga la información al buffer
        display.display(); // Actualiza display con datos en Buffer
        delay(1000); // Demora de 2 segundos.

        //longitud
        display.clearDisplay(); // Borrar imagen en el OLED
        display.setTextSize(0.5); // Definir Tamaño del Texto
        display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
        display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
        display.println("Lon: "); // Carga la información al buffer
        display.println(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6); // Carga la información al buffer
        display.display(); // Actualiza display con datos en Buffer
        delay(1000); // Demora de 2 segundos.
   }else{
        //Buscando satelite
        display.clearDisplay(); // Borrar imagen en el OLED
        display.setTextSize(0.5); // Definir Tamaño del Texto
        display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
        display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
        display.println("Buscando satelite"); // Carga la información al buffer
        display.println("GPS"); // Carga la información al buffer
        display.display(); // Actualiza display con datos en Buffer
        delay(1000); // Demora de 2 segundos.
   }
 
       gps.stats(&chars, &sentences, &failed);
       Serial.print(" CHARS=");
       Serial.print(chars);
       Serial.print(" SENTENCES=");
       Serial.print(sentences);
       Serial.print(" CSUM ERR=");
       Serial.println(failed);

  /*sensor DHT11*/
  // Esperamos 5 segundos entre medidas
  delay(1000);
 
  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
  // Leemos la temperatura en grados Fahreheit
  float f = dht.readTemperature(true);
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
 
  // Calcular el índice de calor en Fahreheit
  float hif = dht.computeHeatIndex(f, h);
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(t, h, false);
 
  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  
  //humedad
  display.clearDisplay(); // Borrar imagen en el OLED
  display.setTextSize(0.5); // Definir Tamaño del Texto
  display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
  display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
  display.println("Humedad"); // Carga la información al buffer
  display.println(h); // Carga la información al buffer
  display.display(); // Actualiza display con datos en Buffer
  delay(1000); // Demora de 2 segundos.

  //temperatura
  display.clearDisplay(); // Borrar imagen en el OLED
  display.setTextSize(0.5); // Definir Tamaño del Texto
  display.setTextColor(WHITE); // Definir color del texto. (mono=>Blanco)
  display.setCursor(0,10); // Definir posición inicio texto Columna (0) Fila (10)
  display.println("Temperatura"); // Carga la información al buffer
  display.println(t); // Carga la información al buffer
  display.display(); // Actualiza display con datos en Buffer
  delay(1000); // Demora de 2 segundos.

  

  
}
