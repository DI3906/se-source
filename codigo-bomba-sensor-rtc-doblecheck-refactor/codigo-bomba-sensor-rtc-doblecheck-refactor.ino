#include <Wire.h>
#include <RTClib.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

//Credenciales wifi
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

//Credenciales BD Firebase
#define FIREBASE_HOST "https://aquabomb-a269e-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "ln4p6QPJBfy0VqmKXcJiBrVcBOGAWq9cv9quZlNr"

RTC_DS3231 rtc;

int led1 = 10;
int led2 = 11;
int trigger = 5; // salida de onda
int echo = 6;    // retorno de la onda
long distancia;
long tiempo;
int motor = 3;

// Fecha y hora que se establece 
const int year = 2024;
const int month = 3;
const int day = 5;
const int hour = 11;
const int minute = 45;
const int second = 0;

// Fecha que se va a comparar
const int year2 = 2024;
const int month2 = 3;
const int day2 = 5;
const int hour2 = 11;
const int minute2 = 47;
const int second2 = 0;

void setup() {
  Serial.begin(9600);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trigger, LOW);
  pinMode(motor, OUTPUT);
  analogWrite(motor, 0);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(2024, 3, 5, 11, 40, 0));
  }

  // Establecer la fecha y hora
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
}

void loop() {
  DateTime now = rtc.now();
  imprimirFechaHora(now);

  // Verificar la distancia solo si la fecha coincide
  if (now.year() == year2 && now.month() == month2 && now.day() == day2 &&
      now.hour() == hour2 && now.minute() == minute2) {
    checkTinacoStatus();
  }
}

void imprimirFechaHora(DateTime datetime) {
  Serial.print(datetime.year(), DEC);
  Serial.print('/');
  Serial.print(datetime.month(), DEC);
  Serial.print('/');
  Serial.print(datetime.day(), DEC);
  Serial.print(" ");
  Serial.print(datetime.hour(), DEC);
  Serial.print(':');
  Serial.print(datetime.minute(), DEC);
  Serial.print(':');
  Serial.print(datetime.second(), DEC);
  Serial.println();
}

void checkTinacoStatus() {
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  tiempo = pulseIn(echo, HIGH); // ASIGNA EL TIEMPO
  distancia = tiempo / 59;
  delay(100);

  if (distancia <= 5) {
    digitalWrite(motor, LOW);
    Serial.println(distancia);
    Serial.println("El tinaco esta lleno");
  } else {
    if (distancia >= 5)
      digitalWrite(motor, HIGH);
    Serial.println(distancia);
    Serial.println("El tinaco esta vacio");
  }
}
