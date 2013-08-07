#include <IRremote.h> //https://github.com/shirriff/Arduino-IRremote
int RECV_PIN = 2;
IRsend irsend;    // For Funky v2 the sending pin is D13 by default
IRrecv irrecv(RECV_PIN);
decode_results results;
// SEND frequency in kHz repeats [pulse widths]. (<<MIND THE PERIOD!)

//SONY on/off
//SEND 38 3 2450 600 1250 550 650 550 1300 550 650 550 1300 500 700 550 650 550 1300 550 650 550 700 550 650 550 700.


//JVC on/off
// SEND 38 1 8500 4150 550 1550 550 1550 550 500 550 500 550 500 550 1550 550 500 600 1500 600 1500 600 1500 600 1500 600 500 550 1550 550 500 550 500 550 500 550 10000 1550 550 1550 550 500 550 500 550 500 550 1550 550 500 600 1500 600 1500 600 1500 600 1500 600 500 550 1550 550 500 550 500 550 500 550 10000 1550 550 1550 550 500 550 500 550 500 550 1550 550 500 600 1500 600 1500 600 1500 600 1500 600 500 550 1550 550 500 550 500 550 500 550 10000 1550 550 1550 550 500 550 500 550 500 550 1550 550 500 600 1500 600 1500 600 1500 600 1500 600 500 550 1550 550 500 550 500 550 500 550.

//SEND 38 3  8450 4150 600 1500 550 1550 600 450 600 450 600 450 600 1500 600 450 600 1500 600 1550 600 1500 550 1550 600 450 600 1500 600 450 600 450 600 450 550.
//SEND 38 1 9050 4450 600 550 550 550 600 500 600 550 600 500 650 500 550 1700 600 500 600 550 550 550 600 550 600 500 550 550 600 550 550 1700 600 500 600 1650 600 550 600 1650 550 1700 600 500 600 550 600 500 600 500 600 550 600 1650 550 550 600 550 600 1650 550 1700 600 1650 550 1700 550.
const uint16_t inputLength = 512;


void setup() {
  Serial.begin(9600);
  Serial.println("READY");
  irrecv.enableIRIn(); // Start the receiver  
}

void loop()
{
    if (irrecv.decode(&results)) {
      dump(&results);
      irrecv.resume(); // Receive the next value
    }


  if ( Serial.available() > 0 )
  {
    static char input[inputLength];
    static uint16_t i;
    char c = Serial.read();
  
    if ( c != '\r' && c != '\n' && c!= '.' && i < inputLength-1)
      input[i++] = c;
    else
    {            
      input[i] = '\0';
      i = 0;

      uint16_t array[150];
      uint16_t j = 0;
      

      if ( !strncmp(input, "SEND", 4) )
      {
        char* p = input+4;

        while ( (p = strchr(p, ' ')) != NULL && j<sizeof(array))
          array[j++] = strtol(p, &p, 10);

        int freq=array[0];
        int repeats=array[1];
        j-=2;
        
        Serial.print("Frequency:"); Serial.println(freq);
        Serial.print("Repeats:"); Serial.println(repeats);
        
        for(int repcount=0;repcount<repeats;repcount++) {
          irsend.sendRaw(array+2, j, freq);    //38Khz default, may require adjustment for other receivers
          delay(2);
        }
        
        irrecv.enableIRIn(); // Start the receiver  
        irrecv.resume(); // Receive the next value
        
        Serial.print("SENT ");
        Serial.print(j + 1); Serial.print(" edges: ");
        for ( uint8_t i = 2; i < j; i++ )    //offset i by 2 to ignore frequency and repeats count
        {
//          Serial.print ("0x");
          Serial.print (array[i], DEC);
          Serial.print(" ");
        }

        Serial.println();
      }
    }
  }

}

void dump(decode_results *results) {
  int count = results->rawlen;

  Serial.print("GOT ");
  Serial.print(count); Serial.print(" edges: ");
  
  for (int i = 1; i < count; i++) {  //Ignore the first edge
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
      Serial.print(" ");
  }
  Serial.println("");
}



