
// Cal-Eng Leonardo mouse-movement example


const int ledPin = 1;         // Mouse control LED 

int mouseX = 0;
int mouseY = 0;
int mouseCount = 0;

boolean moveActive = true;    // whether or not to control the mouse



void setup() {
  
  pinMode(ledPin, OUTPUT);         // the LED pin  
  
  // Wait five seconds since the HID drivers need a bit of time to re-mount after upload.
  delay(5000);
  
}


//  Move the mouse in the Y axis, negative mouseY = up, postitive mouseY = down
void mouseMoveY(int mouseY) {

    Mouse.release(MOUSE_ALL);
    Mouse.press(1);
    
  for (int delta = 100; delta > 0; delta--) {
    
    Mouse.move(0, mouseY, 0);
    delay(5);
    
  }
  
  
}


//  Move the mouse in the x axis, negative mouseX = left, postitive mouseX = right
void mouseMoveX(int mouseX) {
  
    Mouse.release(MOUSE_ALL);
    Mouse.press(2);
  
  for (int delta = 100; delta > 0; delta--) {
    
    Mouse.move(mouseX, 0, 0);
    delay(5);
    
  }
  
  
}



void loop() {
      
  digitalWrite(ledPin, moveActive);
  
  
  if (moveActive == 1) {
    
    mouseMoveY(1);
    mouseMoveX(-1);
    mouseMoveY(-1);
    mouseMoveX(1);
    
    mouseCount++;
    
    //  Only run the loop five times, then stop!
    if ( mouseCount > 5 ) { 
      moveActive = 0; 
      Mouse.release(MOUSE_ALL);
    }

    }
    

}
