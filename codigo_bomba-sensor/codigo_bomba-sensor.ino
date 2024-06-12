int led1 = 10;
int led2 = 11;
int trigger = 5; //salida de onda
int echo = 6; //retorno de la onda
long distancia;
long tiempo;
int motor = 3;

void setup()
{
  Serial.begin(9600);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trigger, LOW);
  pinMode(motor, OUTPUT);
  analogWrite(motor, 0);
}

void loop()
{
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  tiempo = pulseIn (echo, HIGH); //ASGINA EL TIEMPO
  distancia = tiempo / 59;
  delay(10000);
  
  if(distancia <= 7){
    digitalWrite(motor, LOW);
    Serial.println(distancia);
    Serial.println("El tinaco esta lleno");
  } else {
    if (distancia >= 7);
    digitalWrite(motor, HIGH);
    Serial.println(distancia);
    Serial.println("El tinaco esta vacio");
  }//FIN DEL IF-ELSE
}//FIN DEL LOOP
