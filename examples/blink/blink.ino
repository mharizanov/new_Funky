void setup(){
  Serial.begin(9600);
  pinMode(4,OUTPUT);

}
void loop(){
  while(1){
  digitalWrite(4,LOW);
  delay(1000);
  digitalWrite(4,HIGH);
  delay(1000);
  Serial.print(".");

  }
}
