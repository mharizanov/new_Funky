/*
 * Sketch to transfer data between the serial port and an RFM12B. This
 * sketch can be used to emulate an XBee or Ciseco XRF radio link.
 * You will need to use a pair RFM12B modules with similar software at
 * both ends of the link.
 *
 * Supported hardware:
 *   # Calunium v2.0, v2.1 with ATmega1284P or similar microcontroller.
 *
 *   # RFM12B shield for Raspberry Pi with an ATmega328 or ATmega328P
 *     microcontroller.
 *
 * The firmware size is under 8kB when compiled for the ATtiny84 so it
 * may be possible to run the sketch on the RFM12B to Pi expansion
 * board, http://shop.openenergymonitor.com/raspberry-pi/.
 *
 * Copyright S R Marple, 2013.
 * Released under MIT License, http://opensource.org/licenses/mit-license.php
 */

#include <avr/wdt.h>
#include <RF12sm.h>        // https://github.com/stevemarple/RF12

#include <CircBuffer.h>  // https://github.com/stevemarple/CircBuffer
#include <AsyncDelay.h>  // https://github.com/stevemarple/AsyncDelay
#include <RF12_Stream.h>  //https://github.com/stevemarple/RF12_Stream

#if defined(UBRRH) || defined(UBRR0H) || defined(USBCON)
Stream &mySerial = Serial;
#else
// No hardware serial or USB interface
#define USE_SW_SERIAL
#include <SoftwareSerial.h>
#endif

#ifdef CALUNIUM
// Calunium, use pin mapping for v2.0
#define RFM12B_CS 14
#define RFM12B_IRQ_PIN 6
#define RFM12B_IRQ_NUM 2
#define LED_PIN LED_BUILTIN

#elif defined(__AVR_ATmega32U4__)  //Funky v2 

#define RFM12B_CS 10
#define RFM12B_IRQ_PIN 13
#define RFM12B_IRQ_NUM 0
#define LED_PIN 1

#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
// Mapping used by Jeenode and RFM12B shield for Raspberry Pi.
#define RFM12B_CS 10
#define RFM12B_IRQ_PIN 2
#define RFM12B_IRQ_NUM 0
#define LED_PIN 9

#elif defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44__)
// Assume pin-mapping used for Martin Harizanov's RFM2Pi board
#define RFM12B_CS 1
#define RFM12B_IRQ_PIN 2
#define RFM12B_IRQ_NUM 0
#define LED_PIN 8
#endif

#ifdef USE_SW_SERIAL
SoftwareSerial swSerial(7, 3);
Stream &mySerial = swSerial;
#endif

#if RAMEND >= 4096
// eg ATmega644(P), ATmega1284(P), ATmega1280 or ATmega2560
uint8_t rxBuffer[1024];
uint8_t txBuffer[1024];

#elif RAMEND >= 2048
// eg ATmega328
uint8_t rxBuffer[512];
uint8_t txBuffer[512];

#else
// eg ATtiny84
uint8_t rxBuffer[50];
uint8_t txBuffer[50];
#endif

RF12_Stream rfm12b(rxBuffer, sizeof(rxBuffer),
		   txBuffer, sizeof(txBuffer));


const char *firmwareVersion = "SerialStream-1.0";
unsigned long sendInterval_ms = 2000UL;
AsyncDelay activityDelay;

void setup(void)
{
  MCUSR &= ~(1 << WDRF);
  wdt_disable();
  wdt_enable(WDTO_4S);
  
#ifdef SS
  pinMode(SS, OUTPUT);
#endif

#ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
  for (uint8_t i = 0; i < 10; ++i) {
    wdt_reset();
    digitalWrite(LED_PIN, LOW);
    delay(150);
    digitalWrite(LED_PIN, HIGH);
    delay(150);
  }
#endif
  
#ifdef USE_SW_SERIAL
  swSerial.begin(9600);
#else
  Serial.begin(38400);
#endif

  mySerial.print("Firmware version ");
  mySerial.print(firmwareVersion);
  
  if (rfm12b.begin(RFM12B_CS, RFM12B_IRQ_PIN, RFM12B_IRQ_NUM,
		   1, RF12_868MHZ)) {
  }
  else 
    while (1)
      mySerial.println("RFM12B not found");

#ifdef LED_PIN
  digitalWrite(LED_PIN, LOW);
#endif
  activityDelay.expire();

  wdt_reset();
}

void loop(void)
{
  bool activity = false;
  rfm12b.poll();

  // TODO: wait for a while for RFM12B to catch up?
  while (mySerial.available() && rfm12b.getTxBuffer().getSizeRemaining()) {
    rfm12b.write(char(mySerial.read()));
    activity = true;
  }
  
  while (rfm12b.available()) {
    mySerial.write(char(rfm12b.read()));
    activity = true;
  }
  
#ifdef LED_PIN
  if (activityDelay.isExpired()) {
    if (activity) {
      digitalWrite(LED_PIN, HIGH);
      activityDelay.start(250, AsyncDelay::MILLIS);
    }
    else
      digitalWrite(LED_PIN, LOW);
  }
#endif
  
  wdt_reset();
}

