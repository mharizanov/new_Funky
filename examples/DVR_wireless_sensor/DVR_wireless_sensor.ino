/*
 
 Scan RFM12b wireless transmissions and intercept room temperature packets. Raise alarm event to my DVR system in case any room temperature is above 40 degrees C.
 Code is pin specific for Funky v3 
 */

#include <JeeLib.h>

#include <RTClib.h>                 // Real time clock (RTC) - used for software RTC to reset kWh counters at midnight
#include <Wire.h>                   // Part of Arduino libraries - needed for RTClib
RTC_Millis RTC;

//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 5             // Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     // frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 210            // network group, must be same as emonTx and emonBase

#define alarmpin 2

#define _TXLEDINIT	        DDRD  |= (1<<5);
#define _TXLED1			PORTD |= (1<<5);
#define _TXLED0			PORTD &= ~(1<<5);

//---------------------------------------------------
// Data structures for transfering data between units
//---------------------------------------------------

typedef struct { byte magic, hour, mins, sec; } PayloadBase;			// new payload def for time data reception
PayloadBase emonbase; 

typedef struct { int temperature, dummy1,dummy2; } PayloadFunkyOLD;
PayloadFunkyOLD emonfunkyOLD;

typedef struct { int dummy1, temperature,dummy2; } PayloadFunky;
PayloadFunky emonfunky;

int hour = 0, minute = 0, second=0;

void setup() {

    _TXLEDINIT
    _TXLED1
  
    pinMode(alarmpin,OUTPUT);
    digitalWrite(alarmpin,LOW);
    
    pinMode(4,OUTPUT);
    digitalWrite(4,LOW); //Start RFM12b
    
    delay(1000);
        
    rf12_initialize(MYNODE, freq,group);
    
    _TXLED0

delay(3000);       
}

void loop() {

  if (rf12_recvDone())
  {
    if (rf12_crc == 0 )  // and no rf errors
    {
      _TXLED1 //Blink LED to indicate packet received
      
      byte node_id = (rf12_hdr & 0x1F);
      
      
      if (node_id == 22)  // My base station ID, watch out for time packets and set the current time accordingly
      {  
       emonbase = *(PayloadBase*) rf12_data;                           
       RTC.begin(DateTime(2014, 1, 1, emonbase.hour, emonbase.mins, emonbase.sec));
      } 
      
      // Nodes using the new packet format i.e. battery, temperature
      //node 19=bedroom
      //node 9= living room      
      if (node_id == 9 || node_id == 19) {
        emonfunky = *(PayloadFunky*) rf12_data; 
        if (emonfunky.temperature > 4000) alarmevent();
        Serial.print("Node: "); Serial.print(node_id); Serial.print("; Temperature: "); Serial.println((float)emonfunky.temperature/100);
      }

      // Nodes using the old packet format i.e. temperature, battery
      //node 29 = kitchen
      //node 11= kids room 1
      
      if (node_id == 11 || node_id == 29) {
        emonfunkyOLD = *(PayloadFunkyOLD*) rf12_data; 
        if (emonfunkyOLD.temperature > 4000) alarmevent();
        Serial.print("Node: "); Serial.print(node_id); Serial.print("; Temperature: "); Serial.println((float)emonfunkyOLD.temperature/100);        
      }


    }
    delay(10);  //Make LED blink more visible
    _TXLED0
  }
  
   
/*
    DateTime now = RTC.now();    
    hour = now.hour();
    minute = now.minute();
    second = now.second();
*/
}

void alarmevent() {
  _TXLED1
  digitalWrite(alarmpin,HIGH);
  delay(500);
  digitalWrite(alarmpin,LOW);
  _TXLED0
}
