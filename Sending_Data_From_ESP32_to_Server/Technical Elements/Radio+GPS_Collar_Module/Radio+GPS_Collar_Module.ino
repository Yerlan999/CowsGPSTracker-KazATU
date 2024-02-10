#include <HardwareSerial.h>
#include <TinyGPS++.h>

#define M0 18       
#define M1 19

HardwareSerial SerialPort(1); // use UART1

TinyGPSPlus gps;

float latitude;
float longitude;

void setup() {
  SerialPort.begin(9600, SERIAL_8N1, 4, 2); 
  Serial2.begin(9600);   // lora E32 gắn với cổng TX2 RX2 trên board ESP32
  Serial.begin(9600);
  
  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal
  
}

void loop() {

  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        Serial.println(String(latitude) + " | " + String(longitude));
      }
      // else{
      //   Serial.println("Invalid Location");
      // }
    }
  }

    if(Serial.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
      String input = Serial.readStringUntil('\n');
      SerialPort.println(input);     
    }
  
    if(SerialPort.available() > 0){
      String input = SerialPort.readStringUntil('\n');
      Serial.println(input);    
    }
    delay(20);
}
