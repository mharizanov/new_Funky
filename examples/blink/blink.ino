void setup(){
  Serial.begin(9600);
  pinMode(1,OUTPUT);

}
void loop(){
  while(1){
  digitalWrite(1,LOW);
  delay(1000);
  digitalWrite(1,HIGH);
  delay(1000);
  Serial.print(".");

  }
}
