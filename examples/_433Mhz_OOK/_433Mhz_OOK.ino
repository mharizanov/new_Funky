#include <RemoteTransmitter.h> //https://bitbucket.org/fuzzillogic/433mhzforarduino/

RemoteTransmitter RemoteTransmitter(A0,323,8);

void setup() {
  //See example Show_received_code for info on this
pinMode(A0, OUTPUT);
pinMode(2, OUTPUT);
digitalWrite(2,LOW);
}

void loop() {

  unsigned long code;
  unsigned long receivedCode;
  unsigned int period;
 
 //Used this sketch to read the code: https://bitbucket.org/fuzzillogic/433mhzforarduino/src/edabcd754f5b/RemoteSwitch/examples/ShowReceivedCode/ShowReceivedCode.ino?at=default 
  receivedCode=172771;
  period=323;
  
  //Copy the received code. 
  code = receivedCode & 0xFFFFF; //truncate to 20 bits for show; receivedCode is never more than 20 bits..
  
  //Add the period duration to the code. Range: [0..511] (9 bit)
  code |= (unsigned long)period << 23;
  
  //Add the number of repeats to the code. Range: [0..7] (3 bit). The actual number of repeats will be 2^(repeats), 
  //in this case 8
  code |= 3L << 20;

  RemoteTransmitter::sendTelegram(code,A0);
  
  //Wait 5 seconds before sending.
  delay(5000);


  receivedCode=172770;
  period=323;
  
  //Copy the received code. 
  code = receivedCode & 0xFFFFF; //truncate to 20 bits for show; receivedCode is never more than 20 bits..
  
  //Add the period duration to the code. Range: [0..511] (9 bit)
  code |= (unsigned long)period << 23;
  
  //Add the number of repeats to the code. Range: [0..7] (3 bit). The actual number of repeats will be 2^(repeats), 
  //in this case 8
  code |= 3L << 20;

  //Retransmit the signal on pin 11. Note: no object was created!
  RemoteTransmitter::sendTelegram(code,A0);
  delay(5000); 

}
