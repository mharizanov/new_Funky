#include <avr/power.h>
#include <avr/sleep.h>


#include "Ports.h"
#include "RF12.h" // https://github.com/jcw/jeelib
#include "pins_arduino.h"

#define LEDpin 1

#define ACK 1             // Use ACKs?
#define RETRY_PERIOD 1    // How soon to retry (in seconds) if ACK didn't come in
#define RETRY_LIMIT 3    // Maximum number of times to retry
#define ACK_TIME 15       // Number of milliseconds to wait for an ack

#define myNodeID 27      // RF12 node ID in the range 1-30
#define network 210      // RF12 Network group
#define freq RF12_868MHZ // Frequency of RFM12B module


ISR(WDT_vect) { Sleepy::watchdogEvent(); } // interrupt handler for JeeLabs Sleepy power saving

//###############################################################
//Data Structure to be sent
//###############################################################

 typedef struct {
     int temp;	// Temp variable
  	  int supplyV;	// Supply voltage
 } Payload;

 Payload temptx;

int got_interrupt=0;

void setup() {   
    
  pinMode(2,INPUT); // we will wake up from INT1
//  digitalWrite(2,HIGH); // Internal pullup uses way too much power. Use external pull-up ~470K
    
  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,HIGH); 
  
  pinMode(A5,OUTPUT);  //Set RFM12B power control pin (REV 1)
  digitalWrite(A5,LOW); //Start the RFM12B

  powersave();
  
  Sleepy::loseSomeTime(255);  //sleep some more to allor the RFM12b to startup and LED become visible
  rf12_initialize(myNodeID,freq,network); // Initialize RFM12 with settings defined above 
  rf12_control(0xC000);					  // Adjust low battery voltage to 2.2V
  rf12_sleep(0);                          // Put the RFM12 to sleep

  power_spi_disable();   
  
  digitalWrite(LEDpin,LOW);  
  
  got_interrupt=false;
 
}

void loop() {

  digitalWrite(LEDpin,HIGH);  
  power_adc_enable();
  temptx.supplyV = readVcc(); // Get supply voltage
  power_adc_disable();
  digitalWrite(LEDpin,LOW);  
  
  temptx.temp=digitalRead(2);  
  rfwrite(); // Send data via RF       
  
  Sleepy::loseSomeTime(255);  //sleep some more to debounce

  attachInterrupt(1, wake, CHANGE);
  
/*  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();
  sleep_enable();
  interrupts();
  sleep_cpu();
  sleep_disable();
*/

  //Sleep and only wake up from time to time to report battery level OR if the floof sensor reports an event (LOW)
  got_interrupt=false;
  
  for(int j = 1; ((j < 120) && (got_interrupt==false)); j++) {    // Sleep for 120 minutes on until woken by a door open/close event
    Sleepy::loseSomeTime(60000); //JeeLabs power save function: enter low power mode for 60 seconds (valid range 16-65000 ms)
  }


}

void wake() {
got_interrupt=true;
}


//--------------------------------------------------------------------------------------------------
// Send payload data via RF
//--------------------------------------------------------------------------------------------------
static void rfwrite(){
      power_spi_enable();
    
      if(ACK) {
         for (byte i = 0; i <= RETRY_LIMIT; ++i) {  // tx and wait for ack up to RETRY_LIMIT times
           rf12_sleep(-1);              // Wake up RF module
           while (!rf12_canSend())
              rf12_recvDone();
           rf12_sendStart(RF12_HDR_ACK, &temptx, sizeof temptx); 
           rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
           byte acked = waitForAck();  // Wait for ACK
           rf12_sleep(0);              // Put RF module to sleep
           if (acked) {       
             power_spi_disable();        
             return; 
           }       // Return if ACK received
       
        Sleepy::loseSomeTime(RETRY_PERIOD*1000); // If no ack received wait and try again
       }
     }
     else {
  
      rf12_sleep(-1);              // Wake up RF module
      while (!rf12_canSend())
        rf12_recvDone();
      rf12_sendStart(0, &temptx, sizeof temptx); 
      rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
      rf12_sleep(0);              // Put RF module to sleep 
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

  
//--------------------------------------------------------------------------------------------------
// Read current supply voltage
//--------------------------------------------------------------------------------------------------
 long readVcc() {
  byte oldADMUX=ADMUX;  //Save ADC state
  byte oldADCSRA=ADCSRA; 
  byte oldADCSRB=ADCSRB;
  
   long result;
   // Read 1.1V reference against Vcc
//   if(usb==0) clock_prescale_set(clock_div_1);   //Make sure we run @ 8Mhz
   ADCSRA |= bit(ADEN); 
   ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega32u4
   delay(2);
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1126400L / result; // Back-calculate Vcc in mV
   ADCSRA &= ~ bit(ADEN); 
//   if(usb==0) clock_prescale_set(clock_div_2);     
   
   ADCSRA=oldADCSRA; // restore ADC state
   ADCSRB=oldADCSRB;
   ADMUX=oldADMUX;
   return result;
} 
//########################################################################################################################


void powersave() {
  ADCSRA =0;
  ACSR |= (1 << ACD); // disable Analog comparator    
  power_adc_disable();
  power_usart0_disable();
  //power_spi_disable();  /do that a bit later, after we power RFM12b down
  power_twi_disable();
//  power_timer0_disable();
//  power_timer1_disable();
//  power_timer3_disable();
  PRR1 |= (uint8_t)(1 << 4);  //PRTIM4
  power_usart1_disable();
  
  // Switch to RC Clock 
  UDINT  &= ~(1 << SUSPI); // UDINT.SUSPI = 0; Usb_ack_suspend
  USBCON |= ( 1 <<FRZCLK); // USBCON.FRZCLK = 1; Usb_freeze_clock
  PLLCSR &= ~(1 << PLLE); // PLLCSR.PLLE = 0; Disable_pll

  CLKSEL0 |= (1 << RCE); // CLKSEL0.RCE = 1; Enable_RC_clock()
  while ( (CLKSTA & (1 << RCON)) == 0){}	// while (CLKSTA.RCON != 1);  while (!RC_clock_ready())
  CLKSEL0 &= ~(1 << CLKS);  // CLKSEL0.CLKS = 0; Select_RC_clock()
  CLKSEL0 &= ~(1 << EXTE);  // CLKSEL0.EXTE = 0; Disable_external_clock
   
   
   // Datasheet says that to power off the USB interface we have to: 
   //      Detach USB interface 
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
   
   power_usb_disable();  // Keep it here, after the USB power down

}


