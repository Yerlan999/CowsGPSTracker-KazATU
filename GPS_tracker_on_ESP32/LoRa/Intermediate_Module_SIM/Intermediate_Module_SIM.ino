#include <SoftwareSerial.h>

#define M0 12       
#define M1 14

SoftwareSerial mySerial(32, 33); // RX, TX

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  
  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal
  
}

void loop() {
  if(Serial.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
    String input = Serial.readStringUntil('\n');
    mySerial.println(input);
    Serial1.println(input);
    Serial2.println(input);
         
  }
 
  if(mySerial.available() > 0){
    String input = mySerial.readStringUntil('\n');
    Serial.println(input);    
  }
  if(Serial1.available() > 0){
    String input = Serial1.readStringUntil('\n');
    Serial.println(input);    
  }
  if(Serial2.available() > 0){
    String input = Serial2.readStringUntil('\n');
    Serial.println(input);    
  }
  delay(20);
}
