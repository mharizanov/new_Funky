#include "rgb.h"

#define RED_PIN 13
#define GREEN_PIN 8
#define BLUE_PIN 2

#include <SoftPWM.h> //https://code.google.com/p/rogue-code/wiki/SoftPWMLibraryDocumentation

#include <JeeLib.h>

//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 3             // Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     // frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 210            // network group, must be same as emonTx and emonBase

#define ledpin 1

//---------------------------------------------------
// Data structures for transfering data between units
//---------------------------------------------------

typedef struct { int temperature, batt, dummy; } PayloadSolar;
PayloadSolar emonsolar;


void setup() {
  Serial.begin(9600);
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
   
  SoftPWMBegin();  
  
/*

//To test

  for ( double i = -5; i < 40; i+=0.1 ) {
    RGB col = color_for_temperature(i);
    SoftPWMSet(RED_PIN, col.red);
    SoftPWMSet(GREEN_PIN, col.green);
    SoftPWMSet(BLUE_PIN, col.blue);
    delay(10);
  }

  delay(1000);


*/


  pinMode(ledpin,OUTPUT);
  digitalWrite(ledpin,HIGH);
    
  pinMode(A5,OUTPUT);
  digitalWrite(A5,LOW); //Start RFM12b
  delay(100);
  rf12_initialize(MYNODE, freq,group);
  digitalWrite(ledpin,LOW);
    
}


void loop() {
  
   if (rf12_recvDone())
  {
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
    {
      digitalWrite(ledpin,HIGH); //Blink LED to indicate packet received
      byte node_id = (rf12_hdr & 0x1F);      
      if (node_id == 7) {emonsolar = *(PayloadSolar*) rf12_data;      
          double stemp = (double)emonsolar.temperature / 100; 
          RGB col = color_for_temperature(stemp);
          SoftPWMSet(RED_PIN, col.red);
          SoftPWMSet(GREEN_PIN, col.green);
          SoftPWMSet(BLUE_PIN, col.blue);
      }
      
      delay(10);  //Make LED blink more visible
      digitalWrite(ledpin,LOW);
    }
  }
  
  
}

