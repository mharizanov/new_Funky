#include <avr/power.h>
#include <avr/sleep.h>

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "pins_arduino.h"

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // interrupt handler for JeeLabs Sleepy power saving

#define myNodeID 27      // RF12 node ID in the range 1-30
#define network 210      // RF12 Network group
#define freq RF12_868MHZ // Frequency of RFM12B module

#define LEDpin 1


void setup() {

  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,HIGH); 
  delay(6000);
  
  Serial.end();
  Serial1.end();
  
  for (int i=0; i<46; i++) {
    pinMode(i, INPUT);
    digita
  }

//  rf12_initialize(myNodeID,freq,network); // Initialize RFM12 with settings defined above 
  // Adjust low battery voltage to 2.2V
 // rf12_control(0xC040);
 // rf12_sleep(0);                          // Put the RFM12 to sleep
       
  power_adc_disable();
  power_usart0_disable();
  power_spi_disable();
  power_twi_disable();
  //Leave timer 0 going for delay() function
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_timer3_disable();
  power_usart1_disable();

   // Datasheet says that to power off the USB interface we have to do 'some' of: 
   //       Detach USB interface 
   //      Disable USB interface 
   //      Disable PLL 
   //      Disable USB pad regulator 

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

   digitalWrite(LEDpin,LOW); 

  ADCSRA = 0;   // shut off ADC
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();
  sleep_enable();
  sleep_cpu();
  
// I expect uA readings here, but get 3mA...
   
}

void loop() {
  
}


