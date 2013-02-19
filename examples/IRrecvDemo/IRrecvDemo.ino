/*
 * IRhashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * This uses the IRremote library: http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * You're better off using real decoding than this technique, but this is
 * useful if you don't have a decoding algorithm.
 *
 * Copyright 2010 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
  irrecv.enableIRIn(); // Start the receiver
  Serial.begin(9600);
}

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
int compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  } 
  else if (oldval < newval * .8) {
    return 2;
  } 
  else {
    return 1;
  }
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 */
unsigned long decodeHash(decode_results *results) {
  unsigned long hash = FNV_BASIS_32;
  for (int i = 1; i+2 < results->rawlen; i++) {
    int value =  compare(results->rawbuf[i], results->rawbuf[i+2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  return hash;
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.print("Received: ");
    Serial.print(results.value, HEX);
    Serial.print(" ");
    Serial.println(decodeHash(&results), HEX); // Do something interesting with this value
    irrecv.resume(); // Resume decoding (necessary!)
  }
}

#define LEDPIN 4
void blink() {
  digitalWrite(LEDPIN, HIGH);
  delay(200);
  digitalWrite(LEDPIN, LOW);
  delay(200);
}  

// Blink the LED the number of times indicated by the Philips remote control
// Replace loop() with this for the blinking LED example.
void blink_example_loop() {
  if (irrecv.decode(&results)) {
    unsigned long hash = decodeHash(&results);
    switch (hash) {
    case 0x322ddc47: // 0 (10)
//      blink(); // fallthrough
//    default:
      Serial.print("Unknown ");
      Serial.println(hash, HEX);    
    }
    irrecv.resume(); // Resume decoding (necessary!)
  }
}


