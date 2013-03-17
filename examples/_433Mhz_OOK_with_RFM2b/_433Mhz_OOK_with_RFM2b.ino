// This example is based in JCW's KAKU RF sockets control code
// 2009-02-21 jc@wippler.nl http://opensource.org/licenses/mit-license.php
 
// Note that 868 MHz RFM12B's can send 433 MHz just fine, even though the RF
// circuitry is presumably not optimized for that band. Maybe the range will
// be limited, or maybe it's just because 868 is nearly a multiple of 433 ?
 
#include <JeeLib.h>
#include <util/parity.h>
 
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
    for (byte i = 0; i < 10; ++i) {
        for (byte bit = 0; bit < 25; ++bit) {
            int on = bitRead(cmd, bit) ? 1056 : 395;
            ookPulse(on, 1313 - on);
        }
        delay(10); // approximate
    }
}
 
void setup() {
    Serial.begin(9600);
    Serial.println("\n[OOK_RF sockets]");
    rf12_initialize(0, RF12_433MHZ);
}
 
void loop() {
    Serial.println("off");
    OOKSend(0b0001010101010100010101000);
    delay(1000);
    Serial.println("on");
//    OOKSend(0b0111010101010100010101000);
    OOKSend(0b0101010101010100010101000);
    delay(1000);
}
