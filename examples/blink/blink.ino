#define LED 13

void setup(){
  pinMode(LED,OUTPUT);
}

void loop(){
  digitalWrite(LED,!digitalRead(LED));
  delay(1000);
}
