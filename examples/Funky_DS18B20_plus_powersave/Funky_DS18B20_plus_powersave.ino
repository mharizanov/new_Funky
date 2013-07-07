//--------------------------------------------------------------------------------------
// Ultra low power test for the Funkyv2; 
// harizanov.com
// GNU GPL V3
//--------------------------------------------------------------------------------------

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "pins_arduino.h"

#include <OneWire.h>   // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h>  // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_371Beta.zip
#define TEMPERATURE_PRECISION 11
#define ASYNC_DELAY 375 // 9bit requres 95ms, 10bit 187ms, 11bit 375ms and 12bit resolution takes 750ms

#define ONE_WIRE_BUS 8  // pad 5 of the Funky2
#define tempPower 2     // Power pin is connected pad 6 on the Funky2


// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

//addresses of sensors, MAX 4!!
byte allAddress [4][8];  // 8 bytes per address

//--------------------------------------------------------------------------------------

#include <avr/power.h>
#include <avr/sleep.h>

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "pins_arduino.h"

#define LEDpin 13

#define RETRY_PERIOD 1    // How soon to retry (in seconds) if ACK didn't come in
#define RETRY_LIMIT 5     // Maximum number of times to retry
#define ACK_TIME 15       // Number of milliseconds to wait for an ack

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // interrupt handler for JeeLabs Sleepy power saving

#include <EEPROM.h>

// ID of the settings block
#define CONFIG_VERSION "mjh" //keep this 3 chars long
#define CONFIG_START 32

struct StoreStruct {
  // This is for mere detection if they are your settings
  char version[4];
  byte freq, network, myNodeID, ACK, sendp;
} storage = {
  CONFIG_VERSION,
  // The default values
  RF12_868MHZ, 210, 27, false, 5
};

static byte value, stack[20], top;


static byte usb;  // Are we powered via the USB? If so, do not disable it

//Data Structure to be sent, it is variable in size and we only send 2+n*2 bytes where n is the number of DS18B20 sensors attached
//########################################################################################################################

 typedef struct {
    int supplyV;	// Supply voltage
	int temp;	// Temperature reading
  	int temp2;	// Temperature 2 reading
  	int temp3;	// Temperature 3 reading
  	int temp4;	// Temperature 4 reading	
 } Payload;

 Payload temptx;

 int numSensors;
 
 static void setPrescaler (uint8_t mode) {
    cli();
    CLKPR = bit(CLKPCE);
    CLKPR = mode;
    sei();
}

//########################################################################################################################

void setup() {   
  // Because of the fuses, we are running @ 1Mhz now.  

  pinMode(4,OUTPUT);  //Set RFM12B power control pin (REV 1)
  digitalWrite(4,LOW); //Start the RFM12B
    
  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,HIGH); 

  loadConfig();

  USBCON = USBCON | B00010000; 

  delay(550);  // Wait at least between 150ms and 550ms (necessary); Slower host like Raspberry Pi needs more time
 
  if (UDINT & B00000001){
      // USB Disconnected; We are running on battery so we must save power
      usb=0;
      powersave();
//      clock_prescale_set(clock_div_1);   //Run at 4Mhz so we can talk to the RFM12B over SPI
  }
  else {
      // USB is connected 
      usb=1;
      clock_prescale_set(clock_div_1);   //Make sure we run @ 8Mhz; not running on battery so go full speed
      for(int i=0;i<10;i++){
          digitalWrite(LEDpin,LOW); 
          delay(50);
          digitalWrite(LEDpin,HIGH); 
          delay(50);
      }

      Serial.begin(57600);  // Pretty much useless on USB CDC, in fact this procedure is blank. Included here so peope don't wonder where is Serial.begin
      showString(PSTR("\n[Funky v2]\n"));   
      showHelp();

      // Wait for configuration for 10 seconds, then timeout and start the sketch
      unsigned long start=millis();
    
      while((millis()-start)<10000) {
      if (Serial.available())
        {
          handleInput(Serial.read());
          start=millis();
        }
      }

      showString(PSTR("\nStarting sketch.."));   
      Serial.flush();  
    }
 
  digitalWrite(LEDpin,LOW);  
 
  rf12_initialize(storage.myNodeID,storage.freq,storage.network); // Initialize RFM12 
  // Adjust low battery voltage to 2.2V
  rf12_control(0xC000);
  rf12_sleep(0);                          // Put the RFM12 to sleep

  power_spi_disable();   
  
  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output
  digitalWrite(tempPower, HIGH); // turn sensor power on
  dodelay(50); // Allow 50ms for the sensor to be ready
  // Start up the library
  sensors.begin(); 
  sensors.setWaitForConversion(false);   
  numSensors=sensors.getDeviceCount(); 
 
  byte j=0;                                        // search for one wire devices and
                                                   // copy to device address arrays.
  while ((j < numSensors) && (oneWire.search(allAddress[j]))) {        
    j++;
    
    //pretty much useless now since we power the DS18b20 on and off via a digital pin, and the power on default is 12bit
    sensors.setResolution(allAddress[j], TEMPERATURE_PRECISION);      // and set the a to d conversion resolution of each.
  }
    
    
  if(usb==1) { 
    Serial.print("NumSensors:"); Serial.println(numSensors); 

  for (byte i=0; i < numSensors; i++) {
    Serial.print("Device ");
    Serial.print(i);  
    Serial.print(": ");                          
    printAddress(allAddress[i]);                  // print address from each device address arry.
  }
  

}  
  dodelay(1000);

}

void loop() {
    
  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output  
  digitalWrite(tempPower, HIGH); // turn DS18B20 sensor on
  dodelay(20);

  for(int j=0;j<numSensors;j++) {
    sensors.setResolution(allAddress[j], TEMPERATURE_PRECISION);      // and set the a to d conversion resolution of each.
  }
    
  sensors.requestTemperatures(); // Send the command to get temperatures  
  dodelay(ASYNC_DELAY); //Must wait for conversion, since we use ASYNC mode
  
                     temptx.temp=(sensors.getTempC(allAddress[0])*100);   if(usb==1) { Serial.print("Temperature 1:"); Serial.println(temptx.temp);}
  if (numSensors>1) { temptx.temp2=(sensors.getTempC(allAddress[1])*100);   if(usb==1) { Serial.print("Temperature 2:"); Serial.println(temptx.temp2);} }
  if (numSensors>2) { temptx.temp3=(sensors.getTempC(allAddress[2])*100);   if(usb==1) { Serial.print("Temperature 3:"); Serial.println(temptx.temp3);} }
  if (numSensors>3) { temptx.temp4=(sensors.getTempC(allAddress[3])*100);   if(usb==1) { Serial.print("Temperature 4:"); Serial.println(temptx.temp4);} }
  digitalWrite(tempPower, LOW); // turn DS18B20 sensor off
  pinMode(tempPower, INPUT); // set power pin for DS18B20 to input before sleeping, saves power
     

  
  digitalWrite(LEDpin,HIGH);   // LED on  
  power_adc_enable();
  temptx.supplyV = readVcc(); // Get supply voltage
  power_adc_disable();
  digitalWrite(LEDpin,LOW);  //LED off
  
  if(usb==1) { Serial.print("SupplyV:"); Serial.println(temptx.supplyV);}

  rfwrite(); // Send data via RF 

  for(int j = 0; j < 1; j++) {    // Sleep for j minutes
    dodelay(storage.sendp*1000); //JeeLabs power save function: enter low power mode for x seconds (valid range 16-65000 ms)
  }
}


void dodelay(unsigned int ms){
    if(usb==0) 
      Sleepy::loseSomeTime(ms); //JeeLabs power save function: enter low power mode for x seconds (valid range 16-65000 ms)
    else 
      delay(ms);    
}


//--------------------------------------------------------------------------------------------------
// Send payload data via RF
//--------------------------------------------------------------------------------------------------
static void rfwrite(){
      power_spi_enable();
    
      if(storage.ACK) {
         for (byte i = 0; i <= RETRY_LIMIT; ++i) {  // tx and wait for ack up to RETRY_LIMIT times
           rf12_sleep(-1);              // Wake up RF module
           while (!rf12_canSend())
              rf12_recvDone();
           rf12_sendStart(RF12_HDR_ACK, &temptx, numSensors*2 + 2); 
           if (usb==0) rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
             else rf12_sendWait(0); 
           byte acked = waitForAck();  // Wait for ACK
           rf12_sleep(0);              // Put RF module to sleep
           if (acked) {       
             power_spi_disable();        
             return; 
           }       // Return if ACK received
       
        if(!usb) Sleepy::loseSomeTime(RETRY_PERIOD*1000); // If no ack received wait and try again
        else delay(RETRY_PERIOD*1000);       
       }
     }
     else {
  
      rf12_sleep(-1);              // Wake up RF module
      while (!rf12_canSend())
        rf12_recvDone();
      rf12_sendStart(0, &temptx, numSensors*2 + 2); 
           if (usb==0) rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
             else rf12_sendWait(0); 
      rf12_sleep(0);              // Put RF module to sleep 
      power_spi_disable();      
     }
}



  static byte waitForAck() {
   MilliTimer ackTimer;
   while (!ackTimer.poll(ACK_TIME)) {
     if (rf12_recvDone() && rf12_crc == 0 &&
        rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | storage.myNodeID))
        return 1;
     }
   return 0;
  }

  
//--------------------------------------------------------------------------------------------------
// Read current supply voltage
//--------------------------------------------------------------------------------------------------
 long readVcc() {
   long result;
   // Read 1.1V reference against Vcc
//   if(usb==0) clock_prescale_set(clock_div_1);   //Make sure we run @ 8Mhz
   ADCSRA |= bit(ADEN); 
   ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega32u4
   dodelay(2);
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1126400L / result; // Back-calculate Vcc in mV
   ADCSRA &= ~ bit(ADEN); 
//   if(usb==0) clock_prescale_set(clock_div_2);     
   return result;
} 
//########################################################################################################################


void powersave() {
  ADCSRA =0;
  power_adc_disable();
  power_usart0_disable();
  //power_spi_disable();  /do that a bit later, after we power RFM12b down
  power_twi_disable();
//  power_timer0_disable();  / /necessary for the DS18B20 library
  power_timer1_disable();
  power_timer3_disable();
  PRR1 |= (uint8_t)(1 << 4);  //PRTIM4
  power_usart1_disable();
  
  // Switch to RC Clock 
  UDINT  &= ~(1 << SUSPI); // UDINT.SUSPI = 0; Usb_ack_suspend
  USBCON |= ( 1 <<FRZCLK); // USBCON.FRZCLK = 1; Usb_freeze_clock
  PLLCSR &= ~(1 << PLLE); // PLLCSR.PLLE = 0; Disable_pll

  CLKSEL0 |= (1 << RCE); // CLKSEL0.RCE = 1; Enable_RC_clock()
  while ( (CLKSTA & (1 << RCON)) == 0){}	// while (CLKSTA.RCON != 1);  while (!RC_clock_ready())
  CLKSEL0 &= ~(1 << CLKS);  // CLKSEL0.CLKS = 0; Select_RC_clock()
  CLKSEL0 &= ~(1 << EXTE);  // CLKSEL0.EXTE = 0; Disable_external_clock
   
   // Datasheet says that to power off the USB interface we have to: 
   //      Detach USB interface 
   //      Disable USB interface 
   //      Disable PLL 
   //      Disable USB pad regulator 

   // Disable the USB interface 
   USBCON &= ~(1 << USBE); 
    
   // Disable the VBUS transition enable bit 
   USBCON &= ~(1 << VBUSTE); 
    
   // Disable the VUSB pad 
   USBCON &= ~(1 << OTGPADE); 
    
   // Freeze the USB clock 
   USBCON &= ~(1 << FRZCLK); 
    
   // Disable USB pad regulator 
   UHWCON &= ~(1 << UVREGE); 
    
   // Clear the IVBUS Transition Interrupt flag 
   USBINT &= ~(1 << VBUSTI); 
    
   // Physically detact USB (by disconnecting internal pull-ups on D+ and D-) 
   UDCON |= (1 << DETACH); 
   
   power_usb_disable();  // Keep it here, after the USB power down

}

void loadConfig() {
  // To make sure there are settings, and they are ours
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2])
    for (unsigned int t=0; t<sizeof(storage); t++)
      *((char*)&storage + t) = EEPROM.read(CONFIG_START + t);
}

void saveConfig() {
  for (unsigned int t=0; t<sizeof(storage); t++)
    EEPROM.write(CONFIG_START + t, *((char*)&storage + t));
}

static void handleInput (char c) {
    if ('0' <= c && c <= '9')
        value = 10 * value + c - '0';
    else if (c == ',') {
        if (top < sizeof stack)
            stack[top++] = value;
        value = 0;
    } else if ('a' <= c && c <='z') {
        showString(PSTR("> "));
        Serial.print((int) value);
        Serial.println(c);
        switch (c) {
            default:
                showHelp();
                break;

             case 'i': // set node id
                  storage.myNodeID = value;
                  saveConfig();
                break;             
            case 'b': // set band: 4 = 433, 8 = 868, 9 = 915
                  value = value == 8 ? RF12_868MHZ :
                          value == 9 ? RF12_915MHZ : RF12_433MHZ;
                  storage.freq =value;
                  saveConfig();
                break;             
            case 'g': // set network group
                  storage.network = value;
                  saveConfig();
                break;
            case 'p': // set sending period
                  storage.sendp = value;
                  saveConfig();
                break;
            case 'a': // set ACK
                  if(value < 2){  // only 1 and 0 allowed
                    storage.ACK = value;
                    saveConfig();
                  }
                break;
                
                
                
        }
        value = top = 0;
        memset(stack, 0, sizeof stack);
    } else if (c > ' ')
        showHelp();

        rf12_initialize(storage.myNodeID,storage.freq,storage.network); // Initialize RFM12 
    
}


char helpText1[] PROGMEM = 
    "\n"
    "Available commands:" "\n"
    "  <nn> i     - set node ID (standard node ids are 1..26)" "\n"
    "  <n> b      - set MHz band (4 = 433, 8 = 868, 9 = 915)" "\n"
    "  <nnn> g    - set network group (default = 210)" "\n"
    "  <n> a      - set ACK flag (1 = request ACK, 0 = do not requst ACK - default)" "\n"
    "  <nnn> p    - set period for sending in seconds ( default = 20 seconds)" "\n"   
    "\n\n This configuration menu will timeout after 30 seconds of inactivity and sketch will start" "\n"       
    "\n"
;

static void showString (PGM_P s) {
    for (;;) {
        char c = pgm_read_byte(s++);
        if (c == 0)
            break;
        if (c == '\n')
            Serial.print('\r');
        Serial.print(c);
    }
}

static void showHelp() {
    showString(helpText1);
    showString(PSTR("\nCurrent configuration:\n"));

    showString(PSTR("NodeID: "));
    Serial.print(storage.myNodeID,DEC);
    showString(PSTR(", Group: "));
    Serial.print(storage.network,DEC);
    showString(PSTR(", Band: "));
    static word bands[4] = { 315, 433, 868, 915 };
    word band = storage.freq;
    Serial.print(bands[band],DEC);
    showString(PSTR(" MHz"));
    showString(PSTR(", ACKs: "));
    Serial.print(storage.ACK,DEC);
    showString(PSTR(", Sending every "));
    Serial.print(storage.sendp,DEC);
    showString(PSTR(" seconds\n"));
}


void printAddress(DeviceAddress addr) {
  byte i;
  for( i=0; i < 8; i++) {                         // prefix the printout with 0x
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');                        // add a leading '0' if required.
      }
      Serial.print(addr[i], HEX);                 // print the actual value in HEX
      if (i < 7) {
        Serial.print(", ");
      }
    }
  Serial.print("\r\n");
}
