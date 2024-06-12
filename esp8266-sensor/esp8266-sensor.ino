#include <Wire.h>
#include <RTClib.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
//Configuracion de WiFi
#define WIFI_SSID "TYO-23"
#define WIFI_PASSWORD "KEY90_90/"
//Configuracion de Firebase
#define API_KEY "AIzaSyDmLy0XXEPZ6pDaHQNOAgHbkajxSZ98DdQ"
#define DATABASE_URL "https://aquabomb-a269e-default-rtdb.firebaseio.com/"
#define USER_EMAIL "diegg066.99@gmail.com"
#define USER_PASSWORD "p@ssword"

#define PIN_TRIG D5
#define PIN_ECHO D6
#define PIN_BOMBA D7

float tiempo;
float distancia;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
RTC_DS3231 rtc;

unsigned long sendDataPrevMillis = 0;
bool estadoBomba = false;
bool programado = false;
String fechaProg;
String horaProg;

// Fecha y hora que se establece 
const int year = 2024;
const int month = 5;
const int day = 28;
const int hour = 11;
const int minute = 59;
const int second = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BOMBA, OUTPUT);
  digitalWrite(PIN_BOMBA, LOW);
  //Conectar a WiFi
  Serial.print("Conectando a WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Conexion establecida con la IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //Inicializar RTC
  if (!rtc.begin()) {
    Serial.println("¡No se pudo encontrar el módulo RTC DS3231!");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("¡Se detectó una pérdida de energía! Estableciendo la hora...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //Establecer la fecha y hora
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  
  //Configurar FIREBASE
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
}

void loop() {
  digitalWrite(PIN_TRIG, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(4);
   
  digitalWrite(PIN_TRIG, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
   
  tiempo = pulseIn(PIN_ECHO, HIGH);
  distancia = tiempo/58.3;

  DateTime now = rtc.now();
  imprimirFecha(now);
  Serial.println(distancia);

  //Leer el estado de la bomba y la programacion desde Firebase
  if(Firebase.ready() && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    
    //Leer estado de la bomba
    String path= "/Motor/Bomba";
    if(Firebase.RTDB.getBool(&fbdo, path)){
      estadoBomba = fbdo.to<bool>();
      Serial.print("Estado de la bomba recuperado de Firebase: ");
      Serial.println(estadoBomba ? "true" : "false");
    } else {
      Serial.println("Error al recuperar el estado de la bomba: " + fbdo.errorReason());
    }

    //Leer el estado de Programado
    path = "/Programado";
    if(Firebase.RTDB.getBool(&fbdo, path)){
      programado = fbdo.to<bool>();
      Serial.print("Estado de Programado recuperado de firebase: ");
      Serial.println(programado ? "true" : "false");
    } else {
      Serial.println("Error al recuperar el estado de Programado: " + fbdo.errorReason());
    }

    //Leer la fecha programada
    path = "/FechaProg/Fecha";
    if(Firebase.RTDB.getString(&fbdo, path)){
      fechaProg = fbdo.to<String>();
      Serial.print("Fecha programada de firebase: ");
      Serial.println(fechaProg);
    } else {
      Serial.println("Error al recuperar la fecha programada: " + fbdo.errorReason());
    }
    //Leer la hora programada
    path = "/FechaProg/Hora";
    if(Firebase.RTDB.getString(&fbdo, path)){
      horaProg = fbdo.to<String>();
      Serial.print("Hora programada de firebase: ");
      Serial.println(horaProg);
    } else {
      Serial.println("Error al recuperar la fecha programada: " + fbdo.errorReason());
    }

    //Enviar la distancia a Firebase
    path = "/Sensor/Ultrasonico";
    if(Firebase.RTDB.setFloat(&fbdo, path, distancia)){
      Serial.println("Datos enviados a Firebase");
    } else {
      Serial.println("Error enviando datos a Firebase: " + fbdo.errorReason());
    }
  }

  //Obtener la fecha y hora actual de RTC
  //DateTime now = rtc.now();
  String fechaActual = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
  String horaActual = String(now.hour()) + ":" + String(now.minute());

  //Controlar la bomba segun distancia, estados de firebase
  if(estadoBomba && distancia > 5){
    digitalWrite(PIN_BOMBA, HIGH);
    Serial.println("Bomba encendida");
  } else if (programado && fechaProg == fechaActual && horaProg == horaActual && distancia > 5){
    digitalWrite(PIN_BOMBA, HIGH);
    Serial.println("Bomba encendida por programacion");
  } else {
    digitalWrite(PIN_BOMBA, LOW);
    Serial.println("Bomba apagada");
  }

  delay(1000);

}

void imprimirFecha(DateTime datetime){
  Serial.print(datetime.year(), DEC);
  Serial.print("/");
  Serial.print(datetime.month(), DEC);
  Serial.print("/");
  Serial.print(datetime.day(), DEC);
  Serial.print(" ");
  Serial.print(datetime.hour(), DEC);
  Serial.print(":");
  Serial.print(datetime.minute(), DEC);
  Serial.print(":");
  Serial.print(datetime.second(), DEC);
  Serial.println();
}
