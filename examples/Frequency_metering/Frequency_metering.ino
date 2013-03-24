#include <FreqPeriodCounter.h>  //http://www.avdweb.nl/arduino/hardware-interfacing/frequency-period-counter.html

#define TIMER_ENABLE_PWM     (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_PWM    (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_INTR    (TIMSK4 = _BV(TOIE4))
#define TIMER_DISABLE_INTR   (TIMSK4 = 0)
#define TIMER_INTR_NAME      TIMER4_OVF_vect
#define TIMER_CONFIG_HZ(val) ({ \
  const uint16_t pwmval = F_CPU / 2 / (val); \
  TCCR4A = (1<<PWM4A); \
  TCCR4B = _BV(CS40); \
  TCCR4C = 0; \
  TCCR4D = (1<<WGM40); \
  TCCR4E = 0; \
  TC4H = pwmval >> 8; \
  OCR4C = pwmval; \
  TC4H = (pwmval / 3) >> 8; \
  OCR4A = (pwmval / 3) & 255; \
})

#define TIMER_PWM_PIN 13

const byte counterPin = 2; 
const byte counterInterrupt = 1; // = pin 2
FreqPeriodCounter counter(counterPin, micros, 0);
 
void setup(void) 
{ Serial.begin(9600); 
  attachInterrupt(counterInterrupt, counterISR, CHANGE);

  TIMER_DISABLE_INTR; 
  pinMode(TIMER_PWM_PIN, OUTPUT);
  digitalWrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it low
  TIMER_CONFIG_HZ(6000);            // Generate 6Khz on D13, use a jumper wire to hook that to D2 where we will count it
  TIMER_ENABLE_PWM;
}
 
void loop(void) 
{ if(counter.ready()) Serial.println(counter.hertz());
}
 
void counterISR()
{ counter.poll();
}
