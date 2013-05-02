//http://harizanov.com/2013/05/low-power-serial-keypad/

#include <SoftwareSerial.h>
#include <avr/power.h>

SoftwareSerial mySerial(8,2); // RX only

void setup(){
  
  clock_prescale_set(clock_div_1);   //Make sure we run @ 8Mhz; not running on battery so go full speed

  //On board LED
  pinMode(1,OUTPUT);
  digitalWrite(1,LOW);
  
  mySerial.begin(9600);
}

void loop(){
if (mySerial.available()) {
    char x=mySerial.read();
    Serial.write(x);
    Serial.flush();
    if(x>='0' && x<='9') { blip(x-'0',100);}
}
}


void blip(int times, int dur) { 
  pinMode(1,OUTPUT); 
  for(int i=0;i<times;i++){
    digitalWrite(1,HIGH);
    delay(dur);
    digitalWrite(1,LOW);
    delay(dur);
  } 
}
