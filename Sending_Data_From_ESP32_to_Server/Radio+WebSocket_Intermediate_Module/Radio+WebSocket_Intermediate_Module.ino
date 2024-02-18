#include <string.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h> 
#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <Preferences.h>

#define M0 4       
#define M1 2

const int DIR = 5;
const int STEP = 18;
const int  steps_per_rev = 200;
const int contact_button = 19;

String send_text = "";
String received_text = "";
String received_websocket_content = "";

// Последовательный цифровой порт для вывода текста на экран
#define Monitor Serial
#define LoRa Serial2

Adafruit_SSD1306 display(128, 64, &Wire, -1);
WebSocketsServer webSocket= WebSocketsServer(80);
Preferences preferences;

String GPS_ENABLE_COMMAND = "START GPS";
String GPS_DISABLE_COMMAND = "STOP GPS";
String TIME_UPDATE_COMMAND = "TIME_UPDATE";

String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

unsigned long lastTime = 0;
unsigned long cycle_time;    // КАЖДЫЕ N секунд

String ssid;  
String password;
const char *host = "192.168.0.12";
const int port = 8000;


void setup() {

  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", ""); 
  password = preferences.getString("password", "");

  cycle_time = preferences.getInt("time");

  
  Monitor.begin(9600);
  LoRa.begin(9600); 

  WiFi.begin(ssid.c_str(), password.c_str());
 
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

  pinMode(contact_button, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Monitor.println("SSD1306 allocation failed");
  }
  delay(2000);

  IPAddress localIP = WiFi.localIP();
  String localIPString = localIP.toString();
  
  displayInfo(send_text, localIPString, received_websocket_content);
}


void loop() {

  // webSocket.sendTXT(0, inputString);
  webSocket.loop();
    
  if(Monitor.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
    String input = Monitor.readStringUntil('\n');
    send_text = input;
    LoRa.println(input);
    displayInfo(send_text, received_text, received_websocket_content);
  }

  if(LoRa.available() > 0){
    String input = LoRa.readStringUntil('\n');
    if (input.length() > 16){
      received_text = input;
      webSocket.sendTXT(0, input);
      Monitor.println(input);
      displayInfo(send_text, received_text, received_websocket_content);    
    }
    
  }

  delay(20);
}



void updateItems(String input){
  
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);

  if (splitStrings[0].equals(TIME_UPDATE_COMMAND)){
    Monitor.println("Old Time: " + String(cycle_time) + " || New Time: " + splitStrings[1]);
    cycle_time = splitStrings[1].toInt();
    preferences.putInt("time", cycle_time); 
    LoRa.println(input); 
  } 
  
  if (splitStrings[0].equals(GPS_ENABLE_COMMAND)){ LoRa.println(input); }
  if (splitStrings[0].equals(GPS_DISABLE_COMMAND)){ LoRa.println(input); }

    
  if (splitStrings[0].equals(GATE_CLOSE_COMMAND)){
    Monitor.println(input);
    moveStepper(0, splitStrings[1].toInt());
  }
  if (splitStrings[0].equals(GATE_OPEN_COMMAND)){
    Monitor.println(input);
    moveStepper(1, splitStrings[1].toInt());
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
    for(;;){
      digitalWrite(STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(STEP, LOW);
      delayMicroseconds(500);

      if (digitalRead(contact_button)){
        Monitor.println("End Reached");
        preferences.putBool(("gate" + String(gate_ID)).c_str(), bool(direction));    
        webSocket.sendTXT(0, "End Reached");
        break;
      }      
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
      
      bool gate1_state = preferences.getBool("gate1");
      bool gate2_state = preferences.getBool("gate2");
      bool gate3_state = preferences.getBool("gate3");
      bool gate4_state = preferences.getBool("gate4");
      bool gate5_state = preferences.getBool("gate5");
      bool gate6_state = preferences.getBool("gate6");
      bool gate7_state = preferences.getBool("gate7");

      int current_timing = preferences.getInt("time");

      String massive_message = String(current_timing) + "|" + String(gate1_state) + "|" + String(gate2_state) + "|" + String(gate3_state) + "|" + String(gate4_state) + "|" + String(gate5_state) + "|" + String(gate6_state) + "|" + String(gate7_state);

      webSocket.sendTXT(0, massive_message);

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