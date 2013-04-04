
#include <OneWire.h>   // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h>  // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_371Beta.zip
#define TEMPERATURE_PRECISION 9
 
#define ONE_WIRE_BUS A0   
#define tempPower 2      


// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

#define LEDpin 1



 int numSensors;
 

void setup() {

  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,LOW);  // LED on
  delay(100);
  digitalWrite(LEDpin,HIGH);   // LED off
    
  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output
  digitalWrite(tempPower, HIGH); // turn sensor power on
  delay(50);
  // Start up the library
  sensors.begin(); 
  numSensors=sensors.getDeviceCount(); 
}

void loop() {
  digitalWrite(LEDpin,LOW);  //LED on for some time, to show we are alive

  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output  
  digitalWrite(tempPower, HIGH); // turn DS18B20 sensor on
  delay(20);
  sensors.requestTemperatures(); // Send the command to get temperatures  
  
  Serial.println(sensors.getTempCByIndex(0));
  digitalWrite(tempPower, LOW); // turn DS18B20 sensor off

  digitalWrite(LEDpin,HIGH);  //LED off
  
  delay(2000);
  
}


