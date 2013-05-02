//--------------------------------------------------------------------------------------
// Ultra low power test for the Funkyv2; Sends an incrementing value and the VCC readout every 10 seconds
// harizanov.com
// GNU GPL V3
//--------------------------------------------------------------------------------------

 /* 
   I run this sketch with the following Atmega32u4 fuses
   low_fuses=0x7f
   high_fuses=0xd8
   extended_fuses=0xcd
   meaning:
   external crystal 8Mhz, start-up 16K CK+65ms; 
   Divide clock by 8 internally; [CKDIV8=0]  (We will start at 1Mhz since BOD level is 2.2V)
   Boot Reset vector Enabled (default address=$0000); [BOOTRST=0]
   Boot flsh size=2048K words
   Serial program downloading (SPI) enabled; [SPIEN=0]
   BOD=2.2V
*/

#include <avr/power.h>
#include <avr/sleep.h>

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "pins_arduino.h"

#define LEDpin 1

#define ACK 0
#define RETRY_PERIOD 1    // How soon to retry (in seconds) if ACK didn't come in
#define RETRY_LIMIT 5     // Maximum number of times to retry
#define ACK_TIME 15       // Number of milliseconds to wait for an ack

#define myNodeID 26

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // interrupt handler for JeeLabs Sleepy power saving

//###############################################################
//Data Structure to be sent
//###############################################################

 typedef struct {
  	  int temp;	// Temp variable
  	  int supplyV;	// Supply voltage
 } Payload;

 Payload temptx;


void setup() {   
  // Because of the fuses, we are running @ 1Mhz now.  
  pinMode(A5,OUTPUT); // RFM12B power control pin
  digitalWrite(A5,HIGH);  //Make sure the RFM12B is off
    
  powersave();
  Sleepy::loseSomeTime(1024);   
  
  while (readVcc() <3200 ) {// Wait for the VCC to fully rise. Due to lower BOD, we are awake at 2.4V
      Sleepy::loseSomeTime(1023);   
  }

  Sleepy::loseSomeTime(4096);   
  
  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,HIGH); 
  Sleepy::loseSomeTime(32);   
  digitalWrite(LEDpin,LOW);  

  Sleepy::loseSomeTime(4096);   

  digitalWrite(A5,LOW);  //Turn the RFM12B on
  clock_prescale_set(clock_div_2);   //Run at 4Mhz so we can talk to the RFM12B over SPI     
  Sleepy::loseSomeTime(255);  //Wait for it to become ready
  power_spi_enable();    
  rf12_initialize(myNodeID,RF12_868MHZ,210); // Initialize RFM12   
  // Adjust low battery voltage to 2.2V
  rf12_control(0xC000);
  rf12_sleep(0);                          // Put the RFM12 to sleep
  power_spi_disable();   

  Sleepy::loseSomeTime(10000);          // Allow some time for power source to recover    


}

void loop() {
  
  digitalWrite(LEDpin,HIGH);  
  temptx.supplyV = readVcc(); // Get supply voltage
  digitalWrite(LEDpin,LOW);  
  
  if (temptx.supplyV > 3000) {// Only send if enough "juice" is available i.e supply V >2.4V
    temptx.temp++;
    rfwrite(); // Send data via RF 
  }

  for(int j = 0; j < 1; j++) {    // Sleep for j minutes
      Sleepy::loseSomeTime(5000); //JeeLabs power save function: enter low power mode for x seconds (valid range 16-65000 ms)
}
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
   long result;
   // Read 1.1V reference against Vcc
   power_adc_enable();   
   ADCSRA |= bit(ADEN); 
   ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega32u4
   Sleepy::loseSomeTime(16);
   //delay(2);
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1126400L / result; // Back-calculate Vcc in mV
   ADCSRA &= ~ bit(ADEN); 
   power_adc_disable();   
   return result;
} 
//########################################################################################################################


void powersave() {
  ADCSRA =0;
  power_adc_disable();
  power_usart0_disable();
  power_spi_disable();  
  power_twi_disable();
  power_timer0_disable();
  power_timer1_disable();
  power_timer3_disable();
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


