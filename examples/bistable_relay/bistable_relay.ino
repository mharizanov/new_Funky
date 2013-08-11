// Try out Ciseco's bistable relay http://shop.ciseco.co.uk/3v-to-5v-bistable-latching-relay-kit
// Connect D8 to SET, D2 to RESET

#define LED 1
#define SET 2
#define RESET 8
#define rsdelay 10
#include <avr/power.h>


void setup(){
  clock_prescale_set(clock_div_1);   //Make sure we run @ 8Mhz; not running on battery so go full speed

  //On board LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);

  //Relay driving pins
  pinMode(SET,OUTPUT);
  digitalWrite(SET,LOW);
  pinMode(RESET,OUTPUT);
  digitalWrite(RESET,LOW);
  
  
}
void loop() {
  
  
  digitalWrite(SET,HIGH);
  digitalWrite(LED,HIGH);
  delay(rsdelay);
  digitalWrite(SET,LOW);
  digitalWrite(LED,LOW);
  
  delay(1000);
  
  digitalWrite(RESET,HIGH);
  digitalWrite(LED,HIGH);
  delay(rsdelay);
  digitalWrite(RESET,LOW);
  digitalWrite(LED,LOW);
  
  delay(1000);  
  
}

