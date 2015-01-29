#include <IRremote.h> //https://github.com/shirriff/Arduino-IRremote

#define RECEIVER    //RECEIVER OR SENDER; Receiver receives IR and transmits the code wirelessly over to the SENDER that emits it

#ifdef SENDER
IRsend irsend;    // For Funky v2 the sending pin is D13 by default
#else
int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;
//SEND 9050 4450 600 550 550 550 600 500 600 550 600 500 650 500 550 1700 600 500 600 550 550 550 600 550 600 500 550 550 600 550 550 1700 600 500 600 1650 600 550 600 1650 550 1700 600 500 600 550 600 500 600 500 600 550 600 1650 550 550 600 550 600 1650 550 1700 600 1650 550 1700 550.
#endif

const uint16_t inputLength = 512;


void setup() {
  Serial.begin(9600);
  Serial.println("READY");
#ifdef RECEIVER  
  irrecv.enableIRIn(); // Start the receiver  
#endif
}

void loop()
{

#ifdef RECEIVER    
    if (irrecv.decode(&results)) {
      dump(&results);
      irrecv.resume(); // Receive the next value
    }
#else
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

      uint16_t array[80];
      uint16_t j = 0;
      

      if ( !strncmp(input, "SEND", 4) )
      {
        char* p = input+4;

        while ( (p = strchr(p, ' ')) != NULL )
          array[j++] = strtol(p, &p, 10);

        irsend.sendRaw(array, j, 38);    //38Khz default, may require adjustment for other receivers
                
        Serial.print("SENT ");
        Serial.print(j + 1); Serial.print(" edges: ");
        for ( uint8_t i = 0; i < j; i++ )
        {
//          Serial.print ("0x");
          Serial.print (array[i], DEC);
          Serial.print(" ");
        }

        Serial.println();
      }
    }
  }
#endif

}

#ifdef RECEIVER  
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
#endif


