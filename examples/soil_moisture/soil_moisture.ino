//Soil moisture measurement

void setup(){
  pinMode(A8,INPUT);
  digitalWrite(A8,HIGH);  // Internal pull-up, MUST be set!
  Serial.begin(57600);
}
void loop(){
  Serial.println(analogRead(A0));
  delay(500);
}
