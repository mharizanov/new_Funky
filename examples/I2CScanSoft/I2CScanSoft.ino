// Scan I2C bus for device responses

#define SDA_PORT PORTD
#define SDA_PIN 1  //D2
#define SCL_PORT PORTB
#define SCL_PIN 4 //D8
#define I2C_TIMEOUT 100
#define I2C_NOINTERRUPT 0
#define I2C_SLOWMODE 1
#define FAC 1
#define I2C_CPUFREQ (F_CPU/FAC)

#include <SoftI2CMaster.h> //https://github.com/felias-fogg/SoftI2CMaster
#include <avr/io.h>

//------------------------------------------------------------------------------

void setup(void) {

  Serial.begin(9600); 
  Serial.println(F("Intializing ..."));
  Serial.print("I2C delay counter: ");
  Serial.println(I2C_DELAY_COUNTER);
  if (!i2c_init()) 
    Serial.println(F("Initialization error. SDA or SCL are low"));
  else
    Serial.println(F("...done"));
}

void loop(void)
{
  uint8_t add = 0;
  int found = false;
  Serial.println("Scanning ...");

  Serial.println("       8-bit 7-bit addr");
  // try read
  do {
    if (i2c_start(add | I2C_READ)) {
      found = true;
      i2c_read(true);
      i2c_stop();
      Serial.print("Read:   0x");
      if (add < 0x0F) Serial.print(0, HEX);
      Serial.print(add+I2C_READ, HEX);
      Serial.print("  0x");
      if (add>>1 < 0x0F) Serial.print(0, HEX);
      Serial.println(add>>1, HEX);
    } else i2c_stop();
    add += 2;
  } while (add);

  // try write
  add = 0;
  do {
    if (i2c_start(add | I2C_WRITE)) {
      found = true;
      i2c_stop();
      Serial.print("Write:  0x");    
      if (add < 0x0F) Serial.print(0, HEX);  
      Serial.print(add+I2C_WRITE, HEX);
      Serial.print("  0x");
      if (add>>1 < 0x0F) Serial.print(0, HEX);
      Serial.println(add>>1, HEX);
    } else i2c_stop();
    i2c_stop();
    add += 2;
  } while (add);
  if (!found) Serial.println(F("No I2C device found."));
  Serial.println("Done\n\n");
  delay(1000/FAC);
}
