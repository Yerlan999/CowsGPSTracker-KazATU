#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

#define RXD2 33
#define TXD2 32

// HardwareSerial mySerial(0);
SoftwareSerial mySerial(RXD2, TXD2); // RX, TX

#define M0 18       
#define M1 19

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  // mySerial.begin(9600, SERIAL_8E1, RXD2, TXD2); // Set Serial Communication config and reassign pins to 32, 33
  
  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal
  
}
 
void loop() {
    if(Serial.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
      String input = Serial.readStringUntil('\n');
      mySerial.println(input);     
    }
  
    if(mySerial.available() > 0){
      String input = mySerial.readStringUntil('\n');
      Serial.println(input);
    }
    delay(20);
}
