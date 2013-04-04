// Warning only use this for hardware debug!
// See which addresses respond to a start condition.

#include <I2cMaster.h>

// select software or hardware i2c
#define USE_SOFT_I2C 0
#define SDA_PIN A0
#define SCL_PIN 2

SoftI2cMaster rtc(SDA_PIN, SCL_PIN);

#define WIPER_0 0xA9
#define WIPER_1 0xAA
#define WIPER_01 0xAF

//------------------------------------------------------------------------------
void setup(void) {
  delay(10000);
  Serial.begin(9600);
  byte val;
  while(1) {
  rtc.start(0x50 | I2C_WRITE);
  rtc.write(WIPER_01);
  rtc.write(val++);  
  rtc.stop();
  delay(10);
  Serial.println(val);
  }
  
}
void loop(void){}
