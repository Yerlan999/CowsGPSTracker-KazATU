#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <string.h>

#define M0 18       
#define M1 19

HardwareSerial LoRa(1); // use UART1

TinyGPSPlus gps;

#define GPS Serial2
#define Monitor Serial

float latitude;
float longitude;

void setup() {
  LoRa.begin(9600, SERIAL_8N1, 4, 2); 
  GPS.begin(9600);   // lora E32 gắn với cổng TX2 RX2 trên board ESP32
  Monitor.begin(9600);
  
  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal
  
}

void loop() {

  while (GPS.available() > 0) {
    if (gps.encode(GPS.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        Monitor.println(String(latitude) + " | " + String(longitude));
      }
      // else{
      //   Monitor.println("Invalid Location");
      // }
    }
  }

    if(Monitor.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
      String input = Monitor.readStringUntil('\n');
      LoRa.println(input);
      LoRa.flush();     
    }
  
    if(LoRa.available() > 0){
      String input = LoRa.readStringUntil('\n');
      clearSerailPortBuffer();
      Monitor.println(input);
      String check = "GET GPS";

      Monitor.println(input.equals(check));
      
      // if (input == "GET GPS"){
      //   Monitor.println("!!!");
      //   LoRa.println("45.54852 56.23658");
      // }    
    }
    delay(20);
}


void clearSerailBuffer(){
  while(Monitor.available())
    Monitor.read();  
}

void clearSerailPortBuffer(){
  while(LoRa.available())
    LoRa.read();  
}