
#define TIMER_ENABLE_PWM     (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_PWM    (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_INTR    (TIMSK4 = _BV(TOIE4))
#define TIMER_DISABLE_INTR   (TIMSK4 = 0)
#define TIMER_INTR_NAME      TIMER4_OVF_vect

#define TIMER4_RESOLUTION 1023UL  // Timer4 is 11 bit

unsigned long pwmPeriod = 0;

void initialize(unsigned long microseconds=1000000) {

   TCCR4A = (1<<PWM4A);  // 
//   TCCR4E = (1<<ENHC4); // this enables the enhanced mode of the atmega32u4's timer 4. it gives one more bit of resolution (without is has just 10-bit)
   TCCR4D = (1<<WGM40); //set it to phase and frequency correct mode
   TCCR4C = 0; 
   setPeriod(microseconds);
  
}

void setPwmDuty(unsigned int duty) {
	unsigned long dutyCycle = pwmPeriod;
	dutyCycle *= duty;
	dutyCycle >>= 10;
        TC4H = (dutyCycle) >> 8; 
        OCR4A = (dutyCycle) & 255; 
}
        
void start() {
  TCCR4A |= _BV(COM4A1);
}
void stop()  {
  TCCR4A &= ~(_BV(COM4A1));
}
    
void setPeriod(unsigned long microseconds)  {
unsigned long cycles = (F_CPU / 2000000) * microseconds;
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
	} else   
		clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS41) | _BV(CS40);
		pwmPeriod = TIMER4_RESOLUTION - 1;
	

        TCCR4B = clockSelectBits; 
        TC4H = pwmPeriod >> 8; 
        OCR4C = pwmPeriod; 
    }
void setup() {
pinMode(13, OUTPUT);
digitalWrite(13,HIGH);
delay(1000);
initialize(1000000);
setPwmDuty(512);
start();
}
void loop(){}
