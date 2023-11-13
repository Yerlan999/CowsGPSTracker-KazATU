#include <WiFi.h>
#include <HTTPClient.h> 
#include <TinyGPS++.h>

#define RXD2 16
#define TXD2 17

const char* ssid = "LeChatGarcon";
const char* password = "LeChatGarcon999";

unsigned long lastTime = 0;  
unsigned long timerDelay = 10000;    // КАЖДЫЕ 10 секунд

float latitude;
float longitude;

TinyGPSPlus gps;

void setup() {
 
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2); 
  
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
 
}
 
void loop() {

  while (Serial1.available() > 0) {
 
    if (gps.encode(Serial1.read())) {
 
      if (gps.location.isValid()) {
        
        latitude = gps.location.lat();
        longitude = gps.location.lng();

        // Serial.print("Latitude = ");
        // Serial.print(latitude, 6);
        // Serial.print(" || Longitude = ");
        // Serial.println(longitude, 6);
      }
      else{
        // Serial.println("Location Invalid");
      }
    }
  }
 
 if ((millis() - lastTime) > timerDelay) {
   
   if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
   
     HTTPClient http;   
   
     http.begin("http://192.168.0.15:8000/ajax/");
     http.addHeader("Content-Type", "application/x-www-form-urlencoded");             
     
     String index = "123";
     String postData = "latitude=" + String(latitude, 6) + "&longitude=" + String(longitude, 6) + "&index=" + index;

     int httpResponseCode = http.POST(postData);   //Send the actual POST request
   
     if(httpResponseCode>0){
   
      Serial.println(httpResponseCode); 
   
     }else{
   
      Serial.println("Error on sending POST");
   
     }
   
     http.end();  //Free resources
   
   }else{
   
      Serial.println("Error in WiFi connection");   
   
   }
  lastTime = millis();
  }
}
