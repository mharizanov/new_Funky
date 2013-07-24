//————————————————————————————–
// harizanov.com
// GNU GPL V3
//————————————————————————————–

#include <avr/power.h>
#include <avr/sleep.h>

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "pins_arduino.h"

//#define DEBUG //uncomment to enable serial output

#define LEDpin 1

#define RETRY_PERIOD 1 // How soon to retry (in seconds) if ACK didn’t come in
#define RETRY_LIMIT 5 // Maximum number of times to retry
#define ACK_TIME 15 // Number of milliseconds to wait for an ack

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // interrupt handler for JeeLabs Sleepy power saving

#include <EEPROM.h>

#define freq RF12_433MHZ // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int myNodeID = 1; // emonTx RFM12B node ID
const int network = 210;
#define ACK false

static byte value, stack[20], top;

static byte usb; // Are we powered via the USB? If so, do not disable it

//###############################################################
//Data Structure to be sent
//###############################################################

typedef struct { int flowrate, pulse;} PayloadTX;
PayloadTX emontx; // neat way of packaging data for RF comms

// Pulse counting settings
long pulseCount = 0; // Number of pulses, used to measure energy.
unsigned long pulseTime,lastTime; // Used to measure power.

int ppl = 2; // 2 pulses per litre
int secs_delay = 15; // seconds to delay between RX transmissions

int inputPin = 2; // choose the input pin – 3 for Jeenode, 2 for Funky

long lastDebounce = 0;
long debounceDelay = 500; // Ignore bounces under 0.5 second

long lastTX = 0;

void setup() {
// Because of the fuses, we are running @ 1Mhz now.

pinMode(A5,OUTPUT); //Set RFM12B power control pin (REV 1)
digitalWrite(A5,LOW); //Start the RFM12B

pinMode(LEDpin,OUTPUT);
digitalWrite(LEDpin,HIGH);

//loadConfig();

USBCON = USBCON | B00010000;

delay(600); // Wait at least between 150ms and 300ms (necessary); Slower host like Raspberry Pi needs more time

if (UDINT & B00000001){
// USB Disconnected; We are running on battery so we must save power
usb=0;
powersave();
clock_prescale_set(clock_div_2); //Run at 4Mhz so we can talk to the RFM12B over SPI
}
else {
// USB is connected
usb=1;
clock_prescale_set(clock_div_1); //Make sure we run @ 8Mhz; not running on battery so go full speed
for(int i=0;i<10;i++){
digitalWrite(LEDpin,LOW);
delay(50);
digitalWrite(LEDpin,HIGH);
delay(50);
}

Serial.begin(57600); // Pretty much useless on USB CDC, in fact this procedure is blank. Included here so peope don’t wonder where is Serial.begin
showString(PSTR("[Funky v2]\r\n"));

// Wait for configuration for 30 seconds, then timeout and start the sketch
unsigned long start=millis();

showString(PSTR("Starting sketch.."));
Serial.flush();
}

digitalWrite(LEDpin,LOW);

rf12_initialize(myNodeID,freq,network); // Initialize RFM12
// Adjust low battery voltage to 2.2V
rf12_control(0xC000);
rf12_sleep(0); // Put the RFM12 to sleep

power_spi_disable();

//if(!usb) { Sleepy::loseSomeTime(10000); } // Allow some time for power source to recover

pinMode(inputPin, INPUT);
//Set internal pull-up resistor to on
digitalWrite(inputPin,HIGH);
attachInterrupt(1, onPulse, FALLING); // KWH interrupt attached to IRQ 1 = pin3 – hardwired to emonTx pulse jackplug. For connections see: http://openenergymonitor.org/emon/node/208

}

void loop() {

digitalWrite(LEDpin,HIGH);
// power_adc_enable();
// temptx.supplyV = readVcc(); // Get supply voltage
// power_adc_disable();

emontx.pulse = pulseCount;
pulseCount=0;

//Trasnmit data only if pulses have been detected
if (emontx.pulse>0){
rfwrite(); // *SEND RF DATA*
lastTX = millis();
}

//If no pulses detected make sure that a transmit is sent after the last valid transmission
if (emontx.pulse<1 && millis() < (lastTX + ((secs_delay *1000)+1000))){
rfwrite(); // *SEND RF DATA*
}

//If there has been no RF transmission in 1 hour then send data anyway
if (millis() > lastTX + 3600000){
rfwrite(); // *SEND RF DATA*
lastTX = millis();
}
digitalWrite(LEDpin,LOW);

emontx.flowrate = 0;

for(int j = 0; j < 1; j++) { // Sleep for j minutes
if(usb==0)
Sleepy::loseSomeTime(secs_delay*1000); //JeeLabs power save function: enter low power mode for x seconds (valid range 16-65000 ms)
else
delay(secs_delay*1000);
}

}

//————————————————————————————————–
// Send payload data via RF
//————————————————————————————————–
static void rfwrite(){
power_spi_enable();

if(ACK) {
for (byte i = 0; i <= RETRY_LIMIT; ++i) { // tx and wait for ack up to RETRY_LIMIT times
rf12_sleep(-1); // Wake up RF module
while (!rf12_canSend())
rf12_recvDone();
rf12_sendStart(RF12_HDR_ACK, &emontx, sizeof emontx);
rf12_sendWait(2); // Wait for RF to finish sending while in standby mode
byte acked = waitForAck(); // Wait for ACK
rf12_sleep(0); // Put RF module to sleep
if (acked) {
power_spi_disable();
return;
} // Return if ACK received

if(!usb) Sleepy::loseSomeTime(RETRY_PERIOD*1000); // If no ack received wait and try again
else delay(RETRY_PERIOD*1000);
}
}
else {

rf12_sleep(-1); // Wake up RF module
while (!rf12_canSend())
rf12_recvDone();
rf12_sendStart(0, &emontx, sizeof emontx);
rf12_sendWait(2); // Wait for RF to finish sending while in standby mode
rf12_sleep(0); // Put RF module to sleep
power_spi_disable();
}
}

static byte waitForAck() {
MilliTimer ackTimer;
while (!ackTimer.poll(ACK_TIME)) {
if (rf12_recvDone() && rf12_crc == 0 &&
rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | myNodeID))
return 1;
}
return 0;
}

void powersave() {
ADCSRA =0;
power_adc_disable();
power_usart0_disable();
//power_spi_disable(); /do that a bit later, after we power RFM12b down
power_twi_disable();
power_timer0_disable();
power_timer1_disable();
power_timer3_disable();
PRR1 |= (uint8_t)(1 << 4); //PRTIM4
power_usart1_disable();

// Switch to RC Clock
UDINT &= ~(1 << SUSPI); // UDINT.SUSPI = 0; Usb_ack_suspend
USBCON |= ( 1 <<FRZCLK); // USBCON.FRZCLK = 1; Usb_freeze_clock
PLLCSR &= ~(1 << PLLE); // PLLCSR.PLLE = 0; Disable_pll

CLKSEL0 |= (1 << RCE); // CLKSEL0.RCE = 1; Enable_RC_clock()
while ( (CLKSTA & (1 << RCON)) == 0){}	// while (CLKSTA.RCON != 1); while (!RC_clock_ready())
CLKSEL0 &= ~(1 << CLKS); // CLKSEL0.CLKS = 0; Select_RC_clock()
CLKSEL0 &= ~(1 << EXTE); // CLKSEL0.EXTE = 0; Disable_external_clock

// Datasheet says that to power off the USB interface we have to:
// Detach USB interface
// Disable USB interface
// Disable PLL
// Disable USB pad regulator

// Disable the USB interface
USBCON &= ~(1 << USBE);

// Disable the VBUS transition enable bit
USBCON &= ~(1 << VBUSTE);

// Disable the VUSB pad
USBCON &= ~(1 << OTGPADE);

// Freeze the USB clock
USBCON &= ~(1 << FRZCLK);

// Disable USB pad regulator
UHWCON &= ~(1 << UVREGE);

// Clear the IVBUS Transition Interrupt flag
USBINT &= ~(1 << VBUSTI);

// Physically detact USB (by disconnecting internal pull-ups on D+ and D-)
UDCON |= (1 << DETACH);

power_usb_disable(); // Keep it here, after the USB power down

}

static void showString (PGM_P s) {
  for (;;) {
    char c = pgm_read_byte(s++);
    if (c == 0)
      break;
    if (c == '\n')
      Serial.print('\r');
    Serial.print(c);
  }
}

// The interrupt routine – runs each time a falling edge of a pulse is detected
void onPulse()
{
  if( (millis() - lastDebounce) > debounceDelay) {
    lastTime = pulseTime; //used to measure time between pulses.
    pulseTime = micros();
    pulseCount++; //pulseCounter
    emontx.flowrate = int(((60000000.0 / (pulseTime - lastTime))/ppl)*1000); //Calculate flow rate
    lastDebounce = millis();
  }
}

