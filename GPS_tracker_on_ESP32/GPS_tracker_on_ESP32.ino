#include <TinyGPS++.h>

#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;

void setup() {
  Serial.begin(115200, SERIAL_8N1);              
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2); 
}  
 
void loop() {
 
  while (Serial1.available() > 0) {
 
    if (gps.encode(Serial1.read())) {
 
      if (gps.location.isValid()) {
        Serial.print("Latitude   = ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude  = ");
        Serial.println(gps.location.lng(), 6);
      }
      else
        Serial.println("Location Invalid");
 
      if (gps.altitude.isValid()) {
        Serial.print("Altitude   = ");
        Serial.print(gps.altitude.meters());
        Serial.println(" meters");
      }
      else
        Serial.println("Altitude Invalid");
 
      if (gps.speed.isValid()) {
        Serial.print("Speed      = ");
        Serial.print(gps.speed.kmph());
        Serial.println(" kmph");
      }
      else
        Serial.println("Speed Invalid");
 
      if (gps.time.isValid()) {
        Serial.print("Time (GMT) : ");
        if(gps.time.hour() < 10)     Serial.print("0");
        Serial.print(gps.time.hour());
        Serial.print(":");
        if(gps.time.minute() < 10)   Serial.print("0");
        Serial.print(gps.time.minute());
        Serial.print(":");
        if(gps.time.second() < 10)   Serial.print("0");
        Serial.println(gps.time.second());
      }
      else
        Serial.println("Time Invalid");
 
      if (gps.date.isValid()) {
        Serial.print("Date       : ");
        if(gps.date.day() < 10)      Serial.print("0");
        Serial.print(gps.date.day());
        Serial.print("/");
        if(gps.date.month() < 10)    Serial.print("0");
        Serial.print(gps.date.month());
        Serial.print("/");
        Serial.println(gps.date.year());
      }
      else
        Serial.println("Date Invalid");
 
      if (gps.satellites.isValid()) {
        Serial.print("Satellites = ");
        Serial.println(gps.satellites.value());
      }
      else
        Serial.println("Satellites Invalid");
 
    }
 
  }
 
}
