
void setup(){
  pinMode(A8,INPUT);
  pinMode(1,OUTPUT);
  
  digitalWrite(1,HIGH);
  delay(1000);
  digitalWrite(1,LOW);   

  
}

void loop(){
  
  byte detected=0;
  for(int i=0;i<50;i++) { 
  unsigned int curr=analogRead(A8);
  if(curr>0) {
    detected++;
  }
  delay(2);
  }
   if(detected>1) 
     digitalWrite(1,HIGH);
   else
     digitalWrite(1,LOW);   
}

