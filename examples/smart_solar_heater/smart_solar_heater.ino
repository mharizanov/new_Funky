// Maetin's smart hot water tank controler project, check it out here http://harizanov.com/2013/09/smart-iot-solar-hot-water-tank-controller/
#include <OneWire.h>   // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h>  // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_371Beta.zip
#define TEMPERATURE_PRECISION 9
 
#define ONE_WIRE_BUS 13

#include <avr/wdt.h>


// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Connect D8 to SET, D2 to RESET

#define LED 1
#define SET 2
#define RESET 8
#define rsdelay 50
#define QUERY_INTERVAL 30000
#define SEND_INTERVAL 50000
#define TZOFFSET 0

#include <avr/power.h>


#include <JeeLib.h>
//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 7             // Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     // frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 210            // network group, must be same as emonTx and emonBase


#define ACK  0            // Require ACK?
#define RETRY_PERIOD 1    // How soon to retry (in seconds) if ACK didn't come in
#define RETRY_LIMIT 5     // Maximum number of times to retry
#define ACK_TIME 15       // Number of milliseconds to wait for an ack

#define GATEWAYID 16  
#define EMONCMS_CONTROL_FEEDID (int)93; 

#include <RTClib.h>                 // Real time clock (RTC) - used for software RTC to reset kWh counters at midnight
#include <Wire.h>                   // Part of Arduino libraries - needed for RTClib
RTC_Millis RTC;


static byte numSensors;
static byte timeset=0;
static byte relaystatus=0;

void setrelay() {
  
  digitalWrite(SET,HIGH);
  digitalWrite(LED,HIGH);
  delay(rsdelay);
  digitalWrite(SET,LOW);
  digitalWrite(LED,LOW);
  delay(500);
} 


void resetrelay() {  
  
  digitalWrite(RESET,HIGH);
  digitalWrite(LED,HIGH);
  delay(rsdelay);
  digitalWrite(RESET,LOW);
  digitalWrite(LED,LOW);
  delay(500);  
}

typedef struct { int temperature, batt, dummy; } PayloadSolar;
PayloadSolar emonsolar;

typedef struct { byte magic, hour, mins, sec; } PayloadBase;			// new payload def for time data reception
PayloadBase emonbase; 

int hour = 0, minute = 0, second=0;
unsigned long last_emonbase, last_sent, last_gatequery, feed_update;

typedef struct { byte node_id; unsigned int feed_id; float feed_val;} askStruct;			// new payload def for time data reception
askStruct askpayload; 

static int feed_val;


void setup(){
 // clock_prescale_set(clock_div_1);   //Make sure we run @ 8Mhz; not running on battery so go full speed

  wdt_disable();
  //Relay driving pins
  pinMode(SET,OUTPUT);
  pinMode(RESET,OUTPUT);
  resetrelay();

  Serial.begin(9600);

  for (byte i=0;i<10;i++,delay(1000),Serial.print('.'));
  
  //On board LED
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  
  pinMode(A5,OUTPUT);
  digitalWrite(A5,LOW); //Start RFM12b
  delay(500);
  rf12_initialize(MYNODE, freq,group);
  
    // Start up the library
  sensors.begin(); 
  numSensors=sensors.getDeviceCount(); 

  digitalWrite(LED,LOW);
  
  wdt_enable(WDTO_8S); 
  
}
void loop() {
   
  wdt_reset();
     
   if (rf12_recvDone())
  {
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
    {
      digitalWrite(LED,HIGH); //Blink LED to indicate packet received
      
      byte node_id = (rf12_hdr & 0x1F);
      
      if (node_id == GATEWAYID)
      {        
       emonbase = *(PayloadBase*) rf12_data;                           
       
       if(emonbase.magic=='t') { //Valid emonbase time packet?
         RTC.begin(DateTime(2013, 1, 1, emonbase.hour, emonbase.mins, emonbase.sec));
         last_emonbase = millis(); 
         timeset=1;
         Serial.print(F("Time is:"));Serial.print(emonbase.hour);Serial.print(":");Serial.println(emonbase.mins);
       }
       
      }
      
      if (node_id == GATEWAYID)  // Gateway's response to query for emoncms feed value
        {        
         askpayload = *(askStruct*) rf12_data;                                  
           
         if((askpayload.node_id & 0x1F )== GATEWAYID) {
           Serial.print(F("Node id:"));Serial.println(askpayload.node_id);         
           Serial.print(F("Feed id:"));Serial.println(askpayload.feed_id);
           Serial.print(F("Feed val:"));Serial.println(askpayload.feed_val);
           feed_val=(int)askpayload.feed_val;
           feed_update=millis();
         }
         
        } 
      
    delay(10);  //Make LED blink more visible
    digitalWrite(LED,LOW);
  }
  }

  if(millis()-last_sent>SEND_INTERVAL) {

    digitalWrite(LED,HIGH); //Blink LED to indicate packet received 
    sensors.requestTemperatures(); // Send the command to get temperatures    
    emonsolar.temperature=sensors.getTempCByIndex(0)*100;
    emonsolar.batt=readVcc();
    rfwrite();      
    last_sent=millis();
    delay(10);  //Make LED blink more visible
    digitalWrite(LED,LOW);
    Serial.print(F("Temperature is:"));Serial.println(emonsolar.temperature);
  }
 

  if(millis()-last_gatequery>QUERY_INTERVAL) {

    digitalWrite(LED,HIGH); //Blink LED to indicate packet received 

    askpayload.node_id=MYNODE;
    askpayload.feed_id=EMONCMS_CONTROL_FEEDID;
    askpayload.feed_val=0;
    
    byte i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
    rf12_sendStart(RF12_HDR_DST | GATEWAYID, &askpayload, sizeof askpayload);
    rf12_sendWait(0);
    
    Serial.println(F("Sent feed query request"));
    
    last_gatequery=millis();
    delay(10);  //Make LED blink more visible
    digitalWrite(LED,LOW);
       
  }
  
  if(millis()%5000 ==0) controlrelay();
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


void controlrelay() {

Serial.println(millis()-feed_update);

if(millis()-feed_update < 300000L) { //we heard back from emoncms max 5 min ago, so we can use the feed_val as control
Serial.print(F("Feed_val:")); Serial.println(feed_val);

 switch (feed_val) {
    case 0:
      //Off
        if(relaystatus==1) resetrelay();
        relaystatus=0;
      break;
    case 1:
      //On
        if(relaystatus==0) setrelay();
        relaystatus=1;
      break;
    case 2:
      //Auto mode
       autorelay();
      break;
      
    default: 
        if(relaystatus==1) resetrelay();
        relaystatus=0;
  }

} else { // we haven'y heard from emoncms recently, play it safe
        if(relaystatus==1) resetrelay();
        relaystatus=0;
}

}

void autorelay() {
  
    if(timeset) {

    DateTime now = RTC.now();    

    
    int temp = sensors.getTempCByIndex(0);

    // calculate a timezone offset
    DateTime tzadjusted (now.get() + TZOFFSET * 3600);

    byte hour = tzadjusted.hour();
    byte minute = tzadjusted.minute();
    
    Serial.print(F("Time is:"));Serial.print(hour);Serial.print(":");Serial.println(minute);    
    Serial.print(F("Temperature is:"));Serial.println(temp);
    Serial.print(F("Relay status is:"));Serial.println(relaystatus);
    
    if(hour==5 || hour==6) {
      if(temp < 50) {
        if(relaystatus==0) setrelay();
        relaystatus=1;
      }else if(relaystatus==1 && temp > 55+2) {
        resetrelay();
        relaystatus=0;
        }
    } else if(hour==18 || hour==19) {
      if(temp < 55) {
        if(relaystatus==0) setrelay();
        relaystatus=1;
      } else if(relaystatus==1 && temp > 55+5) {
        resetrelay();
        relaystatus=0;
        }
    } else {
        if(relaystatus==1) resetrelay();
        relaystatus=0;
    }
     
  }


}

