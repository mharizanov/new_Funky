#define USBCON 0E0
#define UDINT 0xD8

void setup(){
  USBCON = USBCON | B00010000;
  pinMode(1,OUTPUT); // The on-board LED

  delay(150);  // Wait at least 150ms or else the UDINT always reads TRUE
  
  if (UDINT & B00000001){
  // USB Disconnected code here
       digitalWrite(1,HIGH);
       delay(1000);
       digitalWrite(1,LOW);
  }
  else {
      // USB is connected code here
     while(1) {
       digitalWrite(1,HIGH);
       delay(100);
       digitalWrite(1,LOW);
       delay(100);
    }
  }
  
}

void loop(){

}
