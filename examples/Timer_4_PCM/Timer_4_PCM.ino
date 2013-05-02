#include <avr\pgmspace.h>
#include "coin8.h"


volatile unsigned char *play_sample;
volatile unsigned short play_sample_length;
volatile unsigned short play_sample_ptr;
volatile int play_pin;


#define TIMER4_RESOLUTION 1023UL

#define PLL_FREQ 48000000UL

unsigned long pwmPeriod = 0;

ISR(TIMER4_OVF_vect) {
 // digitalWrite(13,!digitalRead(13));
  
 
  if(play_sample_length)
  {
    setPwmDuty( map(pgm_read_byte(&play_sample[play_sample_ptr]), 0, 255, 0, 1023) );

    play_sample_ptr++;

    if(play_sample_ptr == play_sample_length)
    {
  play_sample_ptr = 0;
  play_sample_length = coin8_len;
    }
  }

}

void enable_intr(){
    TIMSK4 = _BV(TOIE4);
}

void disable_intr(){
    TIMSK4 = 0;
}

void initialize(unsigned long freq) {
  
    /* Init the internal PLL */
  PLLFRQ = _BV(PDIV2);
  PLLCSR = _BV(PLLE);
  while(!(PLLCSR & _BV(PLOCK)));
  PLLFRQ |= _BV(PLLTM0); /* PCK 48MHz */

   TCCR4A = (1<<PWM4A);  
   TCCR4E = (1<<ENHC4); 
   TCCR4D = (1<<WGM40); //set it to phase and frequency correct mode
   TCCR4C = 0; 
   setPeriod(freq);
  
}

void setPwmDuty(unsigned int duty) {
	unsigned long dutyCycle = pwmPeriod;
	dutyCycle *= duty;
	dutyCycle >>= 9;
        TC4H = (dutyCycle) >> 8; 
        OCR4A = (dutyCycle) & 255; 
}
        
void start() {
  TCCR4A |= _BV(COM4A1);
}
void stop()  {
  TCCR4A &= ~(_BV(COM4A1));
}
    
void setPeriod(unsigned long freq)  {
unsigned long cycles = PLL_FREQ / 2 / freq;
unsigned char clockSelectBits = 0;

        if (cycles < TIMER4_RESOLUTION) {
		clockSelectBits = _BV(CS40);
		pwmPeriod = cycles;
	} else
	if (cycles < TIMER4_RESOLUTION * 2) {
		clockSelectBits = _BV(CS41);
		pwmPeriod = cycles / 2;
	} else
	if (cycles < TIMER4_RESOLUTION * 4) {
		clockSelectBits = _BV(CS41) | _BV(CS40);
		pwmPeriod = cycles / 4;
	} else
	if (cycles < TIMER4_RESOLUTION * 8) {
		clockSelectBits = _BV(CS42);
		pwmPeriod = cycles / 8;
	} else
	if (cycles < TIMER4_RESOLUTION * 16) {
		clockSelectBits = _BV(CS42) | _BV(CS40);
		pwmPeriod = cycles / 16;
	} else 
        if (cycles < TIMER4_RESOLUTION * 32) {
		clockSelectBits = _BV(CS42) | _BV(CS41);
		pwmPeriod = cycles / 32;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 64) {
		clockSelectBits = _BV(CS42) | _BV(CS41) | _BV(CS40);
		pwmPeriod = cycles / 64;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 128) {
		clockSelectBits = _BV(CS43);
		pwmPeriod = cycles / 128;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 256) {
		clockSelectBits = _BV(CS43) | _BV(CS40);
		pwmPeriod = cycles / 256;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 512) {
		clockSelectBits = _BV(CS43) | _BV(CS41);
		pwmPeriod = cycles / 512;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 1024) {
		clockSelectBits = _BV(CS43) | _BV(CS41) | _BV(CS40);
		pwmPeriod = cycles / 1024;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 2048) {
		clockSelectBits = _BV(CS43) | _BV(CS42);
		pwmPeriod = cycles / 2048;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 4096) {
		clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS40);
		pwmPeriod = cycles / 4096;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 8192) {
		clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS41);
		pwmPeriod = cycles / 8192;
	} else 	
        if (cycles < TIMER4_RESOLUTION * 16384) {
		clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS41) | _BV(CS40);
		pwmPeriod = cycles / 16384;
	} 

/*else   
		clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS41) | _BV(CS40);
		pwmPeriod = TIMER4_RESOLUTION - 1;
*/	

        TCCR4B = clockSelectBits; 
  
        TC4H = pwmPeriod >> 8; 
        OCR4C = pwmPeriod; 
       
        
    }

void setup() {

play_sample_length = 0;
play_pin = 13;
pinMode(13, OUTPUT);
initialize(8000);
enable_intr();
digitalWrite(13,HIGH);


  
  
//setPwmDuty(512);    //1024/3

start();
//digitalWrite(13,HIGH);

play(coin8, coin8_len);

}
void loop(){
}


void play(const unsigned char *sample, const unsigned short length)
{
  noInterrupts();
  play_sample = (unsigned char *)sample;
  play_sample_ptr = 0;
  play_sample_length = length;
  interrupts();
}

