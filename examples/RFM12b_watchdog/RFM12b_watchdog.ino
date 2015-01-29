#include "RF12mods.h" // https://github.com/jcw/jeelib

#define LEDpin 1

#define myNodeID 27      // RF12 node ID in the range 1-30
#define network 210      // RF12 Network group
#define freq RF12_868MHZ // Frequency of RFM12B module

void setup() {   
    
  delay(1000);
  pinMode(LEDpin,OUTPUT);
  pinMode(8,OUTPUT);  
  digitalWrite(LEDpin,HIGH); 
  
  pinMode(A5,OUTPUT);  //Set RFM12B power control pin (REV 1)
  digitalWrite(A5,LOW); //Start the RFM12B
  delay(100);
  
  rf12_initialize(myNodeID,freq,network); // Initialize RFM12 with settings defined above 

  digitalWrite(LEDpin,LOW);  
  
 
}

void loop() {
  
  if (rf12_recvDone())
  {
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
    {
      digitalWrite(LEDpin,HIGH); //Blink LED to indicate packet received
      
      byte node_id = (rf12_hdr & 0x1F);
      Serial.print(node_id);
            Serial.print(" RSSI: ");
        uint8_t rssi = rf12_getRSSI();
        if (B1000 & rssi) {
            Serial.print(B111 & rssi); // 0 (very poor reception), 2, 4, 6 (very good reception)
        } else { // packet was too short
            Serial.print(rssi); // 1 (bad: 0 or 2), 5 (good: 4 or 6)
          }
        Serial.println();
      digitalWrite(LEDpin,LOW); //Blink LED to indicate packet received
}
  }
}



