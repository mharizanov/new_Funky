
#include <avr/power.h>
#include <avr/sleep.h>


void setup(){

  pinMode(1,OUTPUT);
  digitalWrite(1,HIGH);
  delay(10000);
  digitalWrite(1,LOW);


  ADCSRA =0;
  power_adc_disable();
  power_usart0_disable();
  power_spi_disable();  //do that a bit later, after we power RFM12b down
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

while(1){
   SMCR  =(1<<SM1); //Power down
   SMCR |=(1<<SE); //Enable sleep mode 
   __asm__ __volatile__ ("sleep" "\n\t" :: ); 

  
}
}
void loop(){
}


/*

  Serial.end(); // shut off USB
  
  while(1){
  ADCSRA = 0;   // shut off ADC
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();
  sleep_enable();
  interrupts();
  sleep_cpu();
  sleep_disable();
  }
  
  
  
  
  
  
     
}
*/

