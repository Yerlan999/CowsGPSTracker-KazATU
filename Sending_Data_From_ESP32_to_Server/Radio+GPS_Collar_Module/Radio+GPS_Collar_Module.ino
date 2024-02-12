#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <string.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define M0 18       
#define M1 19

HardwareSerial LoRa(1); // use UART1
Adafruit_SSD1306 display(128, 64, &Wire, -1);
TinyGPSPlus gps;

#define GPS Serial2
#define Monitor Serial

String COW_ID = "1";

float latitude;
float longitude;

String send_text = "";
String received_text = "";

unsigned long lastTime = 0;
unsigned long cycle_time = 10000;    // КАЖДЫЕ N секунд

String GPS_ENABLE_COMMAND = "START GPS";
String GPS_DISABLE_COMMAND = "STOP GPS";
String TIME_UPDATE_COMMAND = "TIME_UPDATE";

bool deliver_GPS = false;


void setup() {
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Monitor.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);

  LoRa.begin(9600, SERIAL_8N1, 4, 2); 
  GPS.begin(9600);   // lora E32 gắn với cổng TX2 RX2 trên board ESP32
  Monitor.begin(9600);
  
  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal
  
  displayInfo(send_text, received_text);
}

void loop() {

    if(Monitor.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
      String input = Monitor.readStringUntil('\n');
      LoRa.println(input);
      send_text = input;
      displayInfo(send_text, received_text); 
    }
  
    if(LoRa.available() > 0){
      String input = LoRa.readStringUntil('\n');
      input.trim();
      Monitor.println(input);
      received_text = input;

      if (input.equals(GPS_ENABLE_COMMAND)){
        deliver_GPS = true;  
      }
      if (input.equals(GPS_DISABLE_COMMAND)){
        deliver_GPS = false;  
      }

      updateTime(input);
      Monitor.println("");
      
      displayInfo(send_text, received_text); 
    }


  if ((millis() - lastTime) > cycle_time) { 
    if (deliver_GPS){
      get_GPS_coordinates();
      LoRa.println(String(COW_ID)+ " | " + String(latitude) + " | " + String(longitude));
      send_text = String(COW_ID)+ " | " + String(latitude) + " | " + String(longitude);
      
      displayInfo(send_text, received_text);
      lastTime = millis();
    }    
  }

  delay(20);   
}


void updateTime(String input){
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);

  if (splitStrings[0].equals(TIME_UPDATE_COMMAND)){
    Monitor.println("Old Time: " + String(cycle_time) + " || New Time: " + splitStrings[1]);
    cycle_time = splitStrings[1].toInt();  
  }      
  delete[] splitStrings;  
}

int splitString(const String& input, char delimiter, String*& output) {
  int arraySize = 1;  // Minimum size is 1 for the original string itself
  int startIndex = 0;
  int endIndex = -1;

  // Count the number of occurrences of the delimiter to determine the array size
  while ((endIndex = input.indexOf(delimiter, startIndex)) != -1) {
    arraySize++;
    startIndex = endIndex + 1;
  }

  // Allocate memory for the dynamic array
  output = new String[arraySize];

  // Split the string into the dynamic array
  startIndex = 0;
  endIndex = -1;
  for (int i = 0; i < arraySize; i++) {
    endIndex = input.indexOf(delimiter, startIndex);
    if (endIndex == -1) {
      endIndex = input.length();
    }
    output[i] = input.substring(startIndex, endIndex);
    startIndex = endIndex + 1;
  }

  return arraySize;
}


void get_GPS_coordinates(){
  while (GPS.available() > 0) {
    if (gps.encode(GPS.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        // Monitor.println(String(latitude) + " | " + String(longitude));
      }
      else{
        latitude = 45.54852;
        longitude = 56.23658;
        // Monitor.println("Invalid Location");
        // Monitor.println(String(latitude) + " | " + String(longitude));
      }
    }
  }  
}

void displayInfo(String send_text, String received_text){
  display.clearDisplay();
  
  display.setTextSize(1);   display.setTextColor(WHITE);
   
  display.setCursor(0, 1);  display.println("R| ");
  display.setCursor(20, 1); display.println(received_text);  
  
  display.setCursor(0, 20);  display.println("S| ");
  display.setCursor(20, 20); display.println(send_text);  

  display.display();   
}
