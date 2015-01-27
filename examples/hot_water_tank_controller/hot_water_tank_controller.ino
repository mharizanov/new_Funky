// Maetin's smart hot water tank controler project, check it out here http://harizanov.com/2013/09/smart-iot-solar-hot-water-tank-controller/
#include <OneWire.h>   // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h>  // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_371Beta.zip
#define TEMPERATURE_PRECISION 9
 
#define ONE_WIRE_BUS 2
#include <avr/wdt.h>

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by
unsigned long lastset;

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

#define LED 11
#define SET 8
#define RESET 8

#include <avr/power.h>


#include <JeeLib.h>
//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 16             // Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     // frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 210            // network group, must be same as emonTx and emonBase


#define ACK  0            // Require ACK?
#define RETRY_PERIOD 1    // How soon to retry (in seconds) if ACK didn't come in
#define RETRY_LIMIT 5     // Maximum number of times to retry
#define ACK_TIME 15       // Number of milliseconds to wait for an ack

#define GATEWAYID 22  
#define SEND_INTERVAL 90000

static byte numSensors;
static byte timeset=0;
static byte relaystatus=0;

void setrelay() {
  
  digitalWrite(SET,HIGH);
  digitalWrite(LED,HIGH);
  relaystatus=1;
} 


void resetrelay() {  
  
  digitalWrite(RESET,LOW);
  digitalWrite(LED,LOW);
  relaystatus=0;
}

typedef struct { int temperature, batt, relaystate; } PayloadSolar;
PayloadSolar emonsolar;

typedef struct { byte relaystatus,dummy1,dummy2,dummy3; } relayStruct;			// new payload def for time data reception
relayStruct relaypayload; 

unsigned long last_sent;

void setup(){

  //On board LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);

  delay(1000);

  wdt_disable();
  //Relay driving pins
  pinMode(SET,OUTPUT);
  pinMode(RESET,OUTPUT);
  resetrelay();
   
  pinMode(4,OUTPUT);
  digitalWrite(4,LOW); //Start RFM12b
  delay(500);
  rf12_initialize(MYNODE, freq,group);
  rf12_control(0xC000);

  Serial.begin(9600);

  for (byte i=0;i<10;i++,delay(1000),Serial.print('.'));
  
    // Start up the library
  sensors.begin(); 
  numSensors=sensors.getDeviceCount(); 

  //Initial reading of temperature
  sensors.requestTemperatures(); // Send the command to get temperatures    
  emonsolar.temperature=sensors.getTempCByIndex(0)*100;

  digitalWrite(LED,LOW);
  
  wdt_enable(WDTO_8S); 
  
}
void loop() {
   
  wdt_reset();
     
   if (rf12_recvDone())
      {


  if(rf12_hdr == (RF12_HDR_DST | MYNODE) && rf12_crc == 0) {       
      
     
      if (rf12_len == 2) {
      
      if (RF12_WANTS_ACK) {
          rf12_sendStart(RF12_ACK_REPLY, 0, 0);
        Serial.println(" -> ack");        
      }
       
        digitalWrite(13,HIGH); //Blink LED to indicate packet received
       Serial.println(F("Received packet from base."));
       relaypayload = *(relayStruct*) rf12_data;                             
         
       if(relaypayload.relaystatus == 0) { 
             rf12_sleep(0);                     // Put the RFM12 to sleep
             resetrelay();
             rf12_sleep(-1);                    // Wake up RF module                   
             Serial.print(F("Relay set to:"));Serial.println(relaypayload.relaystatus);         
         }
         else 
         {    
             rf12_sleep(0);                          // Put the RFM12 to sleep
             setrelay();
             rf12_sleep(-1);              // Wake up RF module          
             Serial.print(F("Relay set to:"));Serial.println(relaypayload.relaystatus);         
         }     
         delay(200);
       }
      }
            
    delay(10);  //Make LED blink more visible
    digitalWrite(13,LOW);
  }
 
  
  
  if(millis()-last_sent>SEND_INTERVAL) {

    digitalWrite(13,HIGH); //Blink LED to indicate packet received 
    sensors.requestTemperatures(); // Send the command to get temperatures    
    emonsolar.temperature=sensors.getTempCByIndex(0)*100;
    emonsolar.batt=readVcc();
    emonsolar.relaystate=relaystatus;
    rfwrite();      
    last_sent=millis();
    delay(10);  //Make LED blink more visible
    digitalWrite(13,LOW);
    Serial.print(F("Temperature is:"));Serial.println(emonsolar.temperature);
      }
 
 if(relaystatus==0) {
  digitalWrite(LED,HIGH); 
 }
 else {
   
   if(millis()-lastset>10) {
     lastset=millis();
     // set the brightness:
  analogWrite(LED, brightness);    
  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade: 
  if (brightness == 0 || brightness == 255) {
    fadeAmount = -fadeAmount ; 
  }     
  
 }
 }
 
}

  
//--------------------------------------------------------------------------------------------------
// Read current supply voltage
//--------------------------------------------------------------------------------------------------
 long readVcc() {
  byte oldADMUX=ADMUX;  //Save ADC state
  byte oldADCSRA=ADCSRA; 
  byte oldADCSRB=ADCSRB;
  
   long result;
   // Read 1.1V reference against Vcc
//   if(usb==0) clock_prescale_set(clock_div_1);   //Make sure we run @ 8Mhz
   ADCSRA |= bit(ADEN); 
   ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega32u4
   delay(2);
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1126400L / result; // Back-calculate Vcc in mV
   ADCSRA &= ~ bit(ADEN); 
//   if(usb==0) clock_prescale_set(clock_div_2);     
   
   ADCSRA=oldADCSRA; // restore ADC state
   ADCSRB=oldADCSRB;
   ADMUX=oldADMUX;
   return result;
} 
//########################################################################################################################



//--------------------------------------------------------------------------------------------------
// Send payload data via RF
//--------------------------------------------------------------------------------------------------
static void rfwrite(){
    
      if(ACK) {
         for (byte i = 0; i <= RETRY_LIMIT; ++i) {  // tx and wait for ack up to RETRY_LIMIT times
           while (!rf12_canSend())
              rf12_recvDone();
           rf12_sendStart(RF12_HDR_ACK, &emonsolar, sizeof(emonsolar)); 
           rf12_sendWait(0); 
           byte acked = waitForAck();  // Wait for ACK
           if (acked) {       
             return; 
           }       // Return if ACK received
         delay(RETRY_PERIOD*1000);       
       }
     }
     else {
      while (!rf12_canSend())
         rf12_recvDone();
      rf12_sendStart(0, &emonsolar, sizeof(emonsolar)); 
      rf12_sendWait(0); 
     }
}



  static byte waitForAck() {
   MilliTimer ackTimer;
   while (!ackTimer.poll(ACK_TIME)) {
     if (rf12_recvDone() && rf12_crc == 0 &&
        rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | MYNODE))
        return 1;
     }
   return 0;
  }


