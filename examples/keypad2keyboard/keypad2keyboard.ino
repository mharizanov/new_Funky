#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {6, 12, 8, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 21, 22, 9}; //connect to the column pinouts of the keypad

/*
Leonardo analog to digital mapping:

A0 – D18
A1 – D19
A2 – D20
A3 – D21
A4 – D22
A5 – D23
A6 – D4
A7 – D6
A8 – D8
A9 – D9
A10 – D10
A11 – D12
*/

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Keyboard.begin();  
}

void loop(){
  char key = keypad.getKey();

  if (key != NO_KEY){
    Keyboard.print(key);
  }
}

