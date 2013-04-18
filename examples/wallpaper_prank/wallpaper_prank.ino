//Description: Minimizes all windows to desktop, takes screenshot, disables desktop icons, saves screenshot in %userprofile% and sets as wallpaper

void setup(){
  
  delay(10000);  
  Keyboard.begin();

  //Show desktop
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.write('d');
  Keyboard.release(KEY_LEFT_GUI);  
  delay(300);

  //Printscreen
  Keyboard.write(0xCE);
  delay(300);
  
  //Invoke MENU 
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.write(KEY_F10);
  Keyboard.release(KEY_LEFT_SHIFT);

  delay(300);
  
  //Hide desktop icons
  Keyboard.write('v');
  delay(300);
  Keyboard.write('d');
  delay(300);  

  //Start Paint
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.write('r');
  Keyboard.release(KEY_LEFT_GUI);
  delay(700);
  Keyboard.println("mspaint.exe");
  //Keyboard.press(KEY_RETURN);  
  delay(1000);
  
  //Paste
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.write('v');
  Keyboard.release(KEY_LEFT_CTRL);

  delay(500);

  //Crop
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_SHIFT);  
  Keyboard.write('x');
  Keyboard.release(KEY_LEFT_SHIFT);    
  Keyboard.release(KEY_LEFT_CTRL);

  delay(500);

  //Save
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.write('s');
  Keyboard.release(KEY_LEFT_CTRL);

  delay(500);

  //Save the screenshot
  Keyboard.println("%userprofile%\\a.bmp");
  delay(1500);
  
  //File menu
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.write('f');
  Keyboard.release(KEY_LEFT_ALT);
  delay(400);
  
  //Set the screenshot as wallpaper  
  Keyboard.write('k');  
  delay(300);
  Keyboard.write('f');  
  delay(1000);
  
  //Exit Paint
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.write(KEY_F4);
  Keyboard.release(KEY_LEFT_ALT);
  delay(400);
  
}
void loop(){

delay(7000);  
}

   
