// Try out Ciseco's bistable relay http://shop.ciseco.co.uk/3v-to-5v-bistable-latching-relay-kit
// Connect D8 to SET, D2 to RESET

#include <avr/power.h>


void setup(){
  clock_prescale_set(clock_div_1);   //Make sure we run @ 8Mhz; not running on battery so go full speed

  //On board LED
  pinMode(1,OUTPUT);
  digitalWrite(1,LOW);

  //Relay driving pins
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);
  pinMode(8,OUTPUT);
  digitalWrite(8,LOW);
  
  
}
void loop() {
  
  
  digitalWrite(2,HIGH);
  digitalWrite(1,HIGH);
  delay(300);
  digitalWrite(2,LOW);
  digitalWrite(1,LOW);
  
  delay(1000);
  
  digitalWrite(8,HIGH);
  digitalWrite(1,HIGH);
  delay(300);
  digitalWrite(8,LOW);
  digitalWrite(1,LOW);
  
  delay(1000);  
  
}

