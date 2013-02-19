#include <avr/power.h>
/*
 
   // 
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
   
*/



void setup(){

blip(5,35);
delay(10000);   // Allow time for changing firmware, otherwise if we disable the USB too soon we are stuck and only a re-flash of the bootloader may help
//12.0mA at this point

power_adc_disable();
blip(1,50);
delay(8000); //11.85mA

power_usart0_disable();
blip(1,50);
delay(8000); //11.79mA

power_spi_disable();
blip(1,50);
delay(8000); //11.75mA

power_twi_disable();
blip(1,50);
delay(8000); //11.7mA

 //Leave timer 0 going for delay() function
power_timer1_disable();
blip(1,50);
delay(8000); // 11.6mA

power_timer2_disable();
blip(1,50);
delay(8000); //11.55mA

power_timer3_disable();
blip(1,50);
delay(8000); //11.35mA

power_usart1_disable();
blip(1,50);
delay(8000);  // 11.35mA

power_usb_disable();
blip(1,50);
delay(8000);  //9.97mA


USBCON |= (1 << FRZCLK);             // Freeze the USB Clock              
PLLCSR &= ~(1 << PLLE);              // Disable the USB Clock (PPL) 
USBCON &=  ~(1 << USBE  );           // Disable the USB  
blip(1,50);
delay(8000);  // 5.3mA


// Switch to RC Clock 
UDINT  &= ~(1 << SUSPI); // UDINT.SUSPI = 0; Usb_ack_suspend
USBCON |= ( 1 <<FRZCLK); // USBCON.FRZCLK = 1; Usb_freeze_clock
PLLCSR &= ~(1 << PLLE); // PLLCSR.PLLE = 0; Disable_pll

CLKSEL0 |= (1 << RCE); // CLKSEL0.RCE = 1; Enable_RC_clock()
while ( (CLKSTA & (1 << RCON)) == 0){}	// while (CLKSTA.RCON != 1);  while (!RC_clock_ready())
CLKSEL0 &= ~(1 << CLKS);  // CLKSEL0.CLKS = 0; Select_RC_clock()
CLKSEL0 &= ~(1 << EXTE);  // CLKSEL0.EXTE = 0; Disable_external_clock
blip(1,50);
delay(8000);  // 5.48mA .. hmmm?!
/*
/// Enable External Clock
CLKSEL0 |= (1 << EXTE);	// CKSEL0.EXTE = 1;	// Enable_external_clock(); 
while ( (CLKSTA & (1 << EXTON)) == 0 ){} // while (CLKSTA.EXTON != 1);	// while (!External_clock_ready()); 
CLKSEL0 |= (1 << CLKS);	// CLKSEL0.CLKS = 1;	//	Select_external_clock(); 
PLLCSR |= (1 << PLLE);	// PLLCSR.PLLE = 1;	// Enable_pll(); 
CLKSEL0 &= ~(1 << RCE);	// CLKSEL0.RCE = 0;	// Disable_RC_clock(); 
while ( (PLLCSR & (1 << PLOCK)) == 0){}	// while (PLLCSR.PLOCK != 1);	// while (!Pll_ready()); 
USBCON &= ~(1 << FRZCLK);	// USBCON.FRZCLK = 0;	// Usb_unfreeze_clock(); 
*/

clock_prescale_set(clock_div_2);
blip(1,50); 
}
void loop(){
// 3.9mA
delay(30000);
}

void blip(int times, int dur) { 
  pinMode(4,OUTPUT); 
  for(int i=0;i<times;i++){
    digitalWrite(4,HIGH);
    delay(dur);
    digitalWrite(4,LOW);
    delay(dur);
  } 
}
