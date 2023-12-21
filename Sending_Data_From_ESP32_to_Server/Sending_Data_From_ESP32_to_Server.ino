#include <WiFi.h>
#include <HTTPClient.h> 
#include <TinyGPS++.h>
#include <ArduinoJson.h>

#define RXD2 16
#define TXD2 17

const char* ssid = "LeChatGarcon";
const char* password = "LeChatGarcon999";
const char *host = "192.168.0.12";
const int port = 8000;

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

  // while (Serial1.available() > 0) {
  //   if (gps.encode(Serial1.read())) {
  //     if (gps.location.isValid()) {
  //       latitude = gps.location.lat();
  //       longitude = gps.location.lng();
  //       // Serial.print("Latitude = ");
  //       // Serial.print(latitude, 6);
  //       // Serial.print(" || Longitude = ");
  //       // Serial.println(longitude, 6);
  //     }
  //     else{
  //       // Serial.println("Location Invalid");
  //     }
  //   }
  // }
 
 if ((millis() - lastTime) > timerDelay) {
   
   if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
      sendHttpPostRequest();
   }else{
      Serial.println("Error in WiFi connection");   
   }
  lastTime = millis();
  }
}



void sendHttpPostRequest() {
  HTTPClient http;

  // Create an array of dictionaries
  DynamicJsonDocument jsonDocument(2048);  // Adjust the size based on your data
  JsonArray jsonArray = jsonDocument.to<JsonArray>();

  // Example dictionaries
  JsonObject dict1 = jsonArray.createNestedObject();
  dict1["latitude"] = 54.214284;
  dict1["longitude"] = 69.514049;
  dict1["index"] = "1";

  JsonObject dict2 = jsonArray.createNestedObject();
  dict2["latitude"] = 54.214440;
  dict2["longitude"] = 69.511097;
  dict2["index"] = "2";

  JsonObject dict3 = jsonArray.createNestedObject();
  dict3["latitude"] = 54.213070;
  dict3["longitude"] = 69.511260;
  dict3["index"] = "3";

  // Add more dictionaries as needed...

  // Serialize the JSON object to a string
  String postData;
  serializeJson(jsonDocument, postData);

  // Make the POST request
  http.begin("http://" + String(host) + ":" + String(port) + "/ajax/");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    Serial.println(httpResponseCode);
  } else {
    Serial.println("Error on sending POST");
  }

  http.end();
}


    //  HTTPClient http;   
    //  http.begin("http://192.168.0.15:8000/ajax/");
    //  http.addHeader("Content-Type", "application/x-www-form-urlencoded");             
    //  String index = "123";
    //  String postData = "latitude=" + String(latitude, 6) + "&longitude=" + String(longitude, 6) + "&index=" + index;
    //  int httpResponseCode = http.POST(postData);   //Send the actual POST request
    //  if(httpResponseCode>0){
    //   Serial.println(httpResponseCode); 
    //  }else{
    //   Serial.println("Error on sending POST");
    //  }
    //  http.end();  //Free resources