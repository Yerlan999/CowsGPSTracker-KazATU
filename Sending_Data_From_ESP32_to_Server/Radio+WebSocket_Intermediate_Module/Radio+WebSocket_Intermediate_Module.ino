#include <string.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GyverEncoder.h"
#include <WiFi.h>
#include <HTTPClient.h> 
#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include <Wire.h>

#define M0 4       
#define M1 2

#define CLK 5 // S1
#define DT 18  // S2
#define SW 19  // Key

const int DIR = 34;
const int STEP = 35;
const int  steps_per_rev = 200;

long unsigned stepper_rotation_count = 1;
bool stepper_change = false;

String send_text = "";
String received_text = "";
String received_websocket_content = "";

// Последовательный цифровой порт для вывода текста на экран
#define Monitor Serial
#define LoRa Serial2

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Encoder enc1(CLK, DT, SW, true);
WebSocketsServer webSocket= WebSocketsServer(80);


String GPS_ENABLE_COMMAND = "START GPS";
String GPS_DISABLE_COMMAND = "STOP GPS";
String TIME_UPDATE_COMMAND = "TIME_UPDATE";

String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

unsigned long lastTime = 0;
unsigned long cycle_time = 10000;    // КАЖДЫЕ N секунд

const char * ssid = "";  
const char * password = "";
const char *host = "192.168.0.12";
const int port = 8000;


void setup() {

  Monitor.begin(9600);
  LoRa.begin(9600); 

  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Monitor.println("Connecting to WiFi..");
  }

  Monitor.println("Connected to the WiFi network");
  Monitor.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal

  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Monitor.println("SSD1306 allocation failed");
  }
  delay(2000);
  displayInfo(send_text, received_text, received_websocket_content);
}


void loop() {

  // webSocket.sendTXT(0, inputString);
  webSocket.loop();
  
  enc1.tick(); // обязательная функция отработки. Должна постоянно опрашиваться

  if (enc1.isLeftH()){ // Monitor.println("Left");
    if (stepper_change){
      stepper_rotation_count++;
      displayStepperInfo(String(stepper_rotation_count)); 
    }
  }; // если был поворот направо с удержанием

  if (enc1.isRightH()){ // Monitor.println("Right");
    if (stepper_change){
      if (stepper_rotation_count > 1){
        stepper_rotation_count--; 
        displayStepperInfo(String(stepper_rotation_count)); 
      } 
    }
  }; // если был поворот налево с удержанием
 
  if (enc1.isRelease()){ // Monitor.println("Release");
    stepper_change = !stepper_change;
    if (stepper_change){
      displayStepperInfo(String(stepper_rotation_count)); 
    }
    else{
      displayInfo(send_text, received_text, received_websocket_content);
    }
    
  }; // отпускание кнопки (+ дебаунс)


  if(Monitor.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
    String input = Monitor.readStringUntil('\n');
    send_text = input;
    LoRa.println(input);
    displayInfo(send_text, received_text, received_websocket_content);
  }

  if(LoRa.available() > 0){
    String input = LoRa.readStringUntil('\n');
    received_text = input;
    webSocket.sendTXT(0, input);

    Monitor.println(input);
    displayInfo(send_text, received_text, received_websocket_content);    
  }

  if ((millis() - lastTime) > cycle_time) {
    
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
      // sendHttpPostRequest();
    }else{
      Monitor.println("Error in WiFi connection");   
    }
  lastTime = millis();
  }

  delay(20);
}



void updateItems(String input){
  
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);

  if (splitStrings[0].equals(TIME_UPDATE_COMMAND)){
    Monitor.println("Old Time: " + String(cycle_time) + " || New Time: " + splitStrings[1]);
    cycle_time = splitStrings[1].toInt(); 
    LoRa.println(input); 
  } 
  
  if (splitStrings[0].equals(GPS_ENABLE_COMMAND)){ LoRa.println(input); }
  if (splitStrings[0].equals(GPS_DISABLE_COMMAND)){ LoRa.println(input); }

    
  if (splitStrings[0].equals(GATE_CLOSE_COMMAND)){
    Monitor.println(input);
    moveStepper(1, splitStrings[1].toInt());
  }
  if (splitStrings[0].equals(GATE_OPEN_COMMAND)){
    Monitor.println(input);
    moveStepper(0, splitStrings[1].toInt());
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



void moveStepper(bool direction, int gate_ID){
  digitalWrite(DIR, direction);
  if (gate_ID == 1){
    Monitor.println("Moving Stepper #1");
    for(int i = 0; i<(steps_per_rev*stepper_rotation_count); i++){
      digitalWrite(STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(STEP, LOW);
      delayMicroseconds(500);
    }
    delay(1000);    
  }
  else{
    Monitor.println("Missing Stepper Motor");
  }  
}

void displayInfo(String send_text, String received_text, String received_websocket_content){
  display.clearDisplay();
  
  display.setTextSize(1);   display.setTextColor(WHITE);
   
  display.setCursor(0, 1);  display.println("S| ");
  display.setCursor(20, 1); display.println(send_text);  
  
  display.setCursor(0, 20);  display.println("R| ");
  display.setCursor(20, 20); display.println(received_text);  

  display.setCursor(0, 40);  display.println("WSK| ");
  display.setCursor(30, 40); display.println(received_websocket_content);  

  display.display();   
}

void displayStepperInfo(String text){
  display.clearDisplay();
  
  display.setTextSize(1);   display.setTextColor(WHITE);
  
  display.setCursor(10, 1);  display.println("Stepper rotation|");
  display.setCursor(55, 40); display.println(text);  
  
  display.display();   
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

// Called when websocket server receives any messages
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
//Figure out the type of Websocket Event

char receivedMessage[length + 1];

switch(type) {
    // Client has disconnected
    case WStype_DISCONNECTED:
      Monitor.printf("[%u] Disconnected\n", num);
      break;
    
    //New client has connected to the server
    case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(num);
      Monitor.printf("[%u] Connected from: \n", num);
      Monitor.println(ip.toString());
    }
    break;
    //Echo the text messages
    case WStype_TEXT:
      // Monitor.printf("[%u] Text %s\n", num, payload);
      // webSocket.sendTXT(num, "YOu are the best");

      // Convert the payload to a C-string
      strncpy(receivedMessage, (char*)payload, length);
      receivedMessage[length] = '\0';
      // Call updateItems with the C-string
      updateItems(String(receivedMessage));
      received_websocket_content = String(receivedMessage);

      displayInfo(send_text, received_text, received_websocket_content);
      
      break;
    // For anything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
    break; 
  }
}