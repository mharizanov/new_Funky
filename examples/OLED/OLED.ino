/*
 
 http://harizanov.com/2013/06/miniature-oled-status-display-with-funky-v2/
 
 */
#include <SoftwareSerial.h>
SoftwareSerial mySerial(6, 8); // RX, TX, RX is set to some dummy pin

#include <JeeLib.h>

#include <RTClib.h>                 // Real time clock (RTC) - used for software RTC to reset kWh counters at midnight
#include <Wire.h>                   // Part of Arduino libraries - needed for RTClib
RTC_Millis RTC;

//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 2             // Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     // frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 210            // network group, must be same as emonTx and emonBase

#define ledpin 1

//---------------------------------------------------
// Data structures for transfering data between units
//---------------------------------------------------
typedef struct { int power1, power2, power3, Vrms, x1,x2,x3,x4,x5,x6;} PayloadTX;         // neat way of packaging data for RF comms
PayloadTX emontx;

typedef struct { int temperature; } PayloadGLCD;
PayloadGLCD emonglcd;

typedef struct { byte magic, hour, mins, sec; } PayloadBase;			// new payload def for time data reception
PayloadBase emonbase; 

typedef struct { int temperature, humidity,dummy; } PayloadFunky;
PayloadFunky emonfunky;

typedef struct { int temperature, batt, dummy; } PayloadSolar;
PayloadSolar emonsolar;

int hour = 0, minute = 0;
double usekwh = 0;
double otemp = 0, humi = 0, stemp=0;
double minotemp, maxotemp, minhumi, maxhumi, batt;
//double use_history[7];
//int cval_use;
//byte page = 1;



//-------------------------------------------------------------------------------------------- 
// Flow control
//-------------------------------------------------------------------------------------------- 
unsigned long last_emontx;                   // Used to count time from last emontx update
unsigned long last_emonbase;                   // Used to count time from last emontx update
unsigned long fast_update;

#define _Digole_Serial_UART_  //To tell compiler compile the special communication only, 
//other available is: _Digole_Serial_I2C_ and _Digole_Serial_SPI_
#include "DigoleSerial.h"
//--------UART setup, if you don't use UART, use // to comment following line
DigoleSerialDisp mydisp(&mySerial, 9600); //UART:Arduino UNO: Pin 1(TX)on arduino to RX on module
//--------I2C setup, if you don't use I2C, use // to comment following 2 lines
//#include <Wire.h>
//DigoleSerialDisp mydisp(&Wire,'\x27');  //I2C:Arduino UNO: SDA (data line) is on analog input pin 4, and SCL (clock line) is on analog input pin 5
//--------SPI setup, if you don't use SPI, use // to comment following line
//DigoleSerialDisp mydisp(8,9,10);  //SPI:Pin 8: data, 9:clock, 10: SS, you can assign 255 to SS, and hard ground SS pin on module
#define LCDCol 16
#define LCDRow 2
#define LCDW 240

const unsigned char fonts[] = {6, 10, 18, 51, 120, 123};
const char *fontdir[] = {"0\xb0", "90\xb0", "180\xb0", "270\xb0"};

void setup() {

    pinMode(ledpin,OUTPUT);
    digitalWrite(ledpin,HIGH);
    
    mydisp.begin();
        
    pinMode(A5,OUTPUT);
    digitalWrite(A5,LOW); //Start RFM12b
    
    delay(3000);
    
    /*----------for text LCD adapter and graphic LCD adapter ------------*/
    mydisp.clearScreen(); //CLear screen
    mydisp.disableCursor(); //disable cursor, enable cursore use: enableCursor();
    
    
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
      if (node_id == 10) {emontx = *(PayloadTX*) rf12_data; last_emontx = millis();}
      if (node_id == 20) {
        emonfunky = *(PayloadFunky*) rf12_data; last_emontx = millis();
        otemp = (double)emonfunky.temperature / 100;
        humi = (double)emonfunky.humidity / 100;
      }
      
      if (node_id == 7) {emonsolar = *(PayloadSolar*) rf12_data; last_emontx = millis();     stemp = (double)emonsolar.temperature / 100; }

      
      if (node_id == 22)
      {
        
       emonbase = *(PayloadBase*) rf12_data;                           
       RTC.begin(DateTime(2013, 1, 1, emonbase.hour, emonbase.mins, emonbase.sec));
       last_emonbase = millis(); 
      
      } 
    }
    delay(10);  //Make LED blink more visible
    digitalWrite(ledpin,LOW);
  }


 //--------------------------------------------------------------------------------------------
  // Display update every 5000ms
  //--------------------------------------------------------------------------------------------
  if ((millis()-fast_update)>5000)
  {
    fast_update = millis();
    
    DateTime now = RTC.now();
    int last_hour = hour;
    hour = now.hour();
    minute = now.minute();

    usekwh += (emontx.power1 * 5) / 3600000;
    draw1();
  }
}

void draw1(){
    mydisp.setFont(51);    
    resetpos(0);         
     //mydisp.setTextPosAbs(2, 27);
     
    if(hour<10){mydisp.print(" ");} mydisp.print(hour);     mydisp.print(":");   if(minute<10){mydisp.print("0");} mydisp.println(minute); mydisp.println(" ");
    mydisp.setFont(10);    
    resetpos(4);    
    mydisp.print("Power: ");     mydisp.print(emontx.power1);   mydisp.print("W (");    mydisp.print((int)usekwh); mydisp.print("kWh) "); 
    resetpos(5);    
    mydisp.print("Solar: ");     mydisp.print(stemp);   mydisp.print("C ");    
    resetpos(6);    
    mydisp.print("Out: ");     mydisp.print(otemp);   mydisp.print("C Humi:"); mydisp.print((int)humi);   mydisp.print("% ");    

//    mydisp.drawFrame(0, 0, 127, 64);
}

void resetpos(int pos) 
{
    mydisp.setPrintPos(1, pos, _TEXT_);

}

