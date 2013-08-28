#include <JeeLib.h>
#include <util/parity.h>

int pin = 8;
unsigned long duration,lastheard;
unsigned long timeout = 30000000;
#define LEDpin 1
#define CLAPMIN 300
#define CLAPMAX 600
 
// Turn transmitter on or off, but also apply asymmetric correction and account
// for 25 us SPI overhead to end up with the proper on-the-air pulse widths.
// With thanks to JGJ Veken for his help in getting these values right.
static void ookPulse(int on, int off) {
    rf12_onOff(1);
    delayMicroseconds(on + 150);
    rf12_onOff(0);
    delayMicroseconds(off - 200);
}
 
static void OOKSend(unsigned long cmd) {
    for (byte i = 0; i < 40; ++i) {
        for (byte bit = 0; bit < 25; ++bit) {
            int on = bitRead(cmd, bit) ? 1056 : 395;
            ookPulse(on, 1313 - on);
        }
        delay(10); // approximate
    }
}

void setup()
{
  pinMode(pin, INPUT);
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,LOW); 

  pinMode(A5,OUTPUT);  //Start RFM12b
  digitalWrite(A5,LOW);
  delay(100);

  Serial.begin(9600);
  Serial.println(F("\n[Clap controlled power sockets]"));
  rf12_initialize(0, RF12_433MHZ);

}

void loop()
{
  duration = pulseIn(pin, HIGH, timeout);
  if(duration > 1) {
    Serial.println((millis()-lastheard));
    if((millis()-lastheard>CLAPMIN) && (millis()-lastheard<CLAPMAX)) {
      digitalWrite(LEDpin,!(digitalRead(LEDpin)));
      if(digitalRead(LEDpin))
        OOKSend(0b0111010101010100010101000);
      else
        OOKSend(0b0001010101010100010101000);
    }
      lastheard=millis();
  }

  delay(10); 
//  Serial.println(duration);
}

