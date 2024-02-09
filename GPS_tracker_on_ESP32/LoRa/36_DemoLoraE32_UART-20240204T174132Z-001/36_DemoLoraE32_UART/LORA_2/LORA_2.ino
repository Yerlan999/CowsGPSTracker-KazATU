#include <SoftwareSerial.h>

SoftwareSerial mySerial(6, 7); //TX, RX

#define M0 8
#define M1 9                 

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  
  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal
  
}

void loop() {
  if(Serial.available() > 0){
    String input = Serial.readString();
    mySerial.println(input);    
  }
 
  if(mySerial.available() > 1){
    String input = mySerial.readString();
    Serial.println(input);    
  }
  delay(20);
}
