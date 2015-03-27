
// To use on Funky v3 without Pull-up reistor, check Josh's post and his one wire library port
// http://wp.josh.com/2014/06/23/no-external-pull-up-needed-for-ds18b20-temp-sensor/

#include <OneWire.h>   // https://github.com/bigjosh/OneWireNoResistor/archive/master.zip
#include <DallasTemperature.h>  // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_371Beta.zip
#define TEMPERATURE_PRECISION 9
 
#define ONE_WIRE_BUS 8
#define tempPower 2      


// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

#define LEDpin 13



 int numSensors;
 

void setup() {

  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,HIGH);  // LED on
  delay(100);
  digitalWrite(LEDpin,LOW);   // LED off
    
  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output
  digitalWrite(tempPower, HIGH); // turn sensor power on
  delay(500);
  // Start up the library
  sensors.begin(); 
  numSensors=sensors.getDeviceCount(); 
}

void loop() {
  digitalWrite(LEDpin,HIGH);  //LED on for some time, to show we are alive
  sensors.requestTemperatures(); // Send the command to get temperatures  
  Serial.println(sensors.getTempCByIndex(0));
  digitalWrite(LEDpin,LOW);  //LED off
  delay(2000);
  
}


