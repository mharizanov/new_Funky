#include <DHT22.h> // https://github.com/nathanchantrell/Arduino-DHT22

#define DHT22Pin 8      // DHT Data pin
#define tempPower 2     // Power up/down the DHT22 sensor with this pin to save power

DHT22 dht(DHT22Pin); // Setup the DHT

#define LEDpin 13


void setup() {

  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,HIGH);  // LED on
  delay(100);
  digitalWrite(LEDpin,LOW);   // LED off
    
  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output
  digitalWrite(tempPower, HIGH); // turn sensor power on
  delay(500);
  // Start up the library
}

void loop() {
  digitalWrite(LEDpin,HIGH);  //LED on for some time, to show we are alive

  DHT22_ERROR_t errorCode;  
  errorCode = dht.readData(); // read data from sensor

  if (errorCode == DHT_ERROR_NONE) { // data is good  
     Serial.println(dht.getTemperatureC());
     Serial.println(dht.getHumidity());
  }
  
  digitalWrite(LEDpin,LOW);  //LED off
  delay(4000);  // Read the DHT22 no more often than once every 3 sec
  
}


