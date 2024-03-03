#include <WiFi.h> 
#include <WebSocketsServer.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <Preferences.h>
#include <HardwareSerial.h>
#include "Arduino.h"
#include "LoRa_E32.h"

#define Monitor Serial 

LoRa_E32 LoRa(&Serial2, 15, 22, 21); //  Serial Pins,  AUX, M0, M1
RF24 nRF24(4, 5); // CE, CSN
WebSocketsServer webSocket= WebSocketsServer(80);
Preferences preferences;

const byte address[2][6] = {"00001", "00002"};

const int listen_to = 0;
const int write_into = 1;

String GPS_ENABLE_COMMAND = "START GPS";
String GPS_DISABLE_COMMAND = "STOP GPS";
String TIME_UPDATE_COMMAND = "TIME_UPDATE";
 
String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

String ssid;  
String password;

unsigned long cycle_time;    // КАЖДЫЕ N секунд

void setup(){
  
  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", ""); 
  password = preferences.getString("password", "");

  cycle_time = preferences.getInt("time");

  Monitor.begin(9600); //открываем порт для связи с ПК
  
  LoRa.begin();

  nRF24.begin(); 
  //Append ACK packet from the receiving nRF24 back to the transmitting nRF24 
  nRF24.setAutoAck(true); //(true|false) 
  //Set the transmission datarate 
  nRF24.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS) 
  //Greater level = more consumption = longer distance 
  nRF24.setPALevel(RF24_PA_MIN); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX) 
  //Default value is the maximum 32 bytes1 
  nRF24.setRetries(0, 15);
  nRF24.enableDynamicPayloads();
  nRF24.enableAckPayload();
  nRF24.setPayloadSize(32);
  //Act as receiver 
  nRF24.openReadingPipe(1, address[listen_to]);
  nRF24.openWritingPipe(address[write_into]); 
  
  nRF24.startListening();

  WiFi.begin(ssid.c_str(), password.c_str());
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Monitor.println("Connecting to WiFi..");
  }

  Monitor.println("Connected to the WiFi network");
  Monitor.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  
  webSocket.loop();

  listenToNRF24();

  if (Monitor.available()){
    
    char message_send[32]; // Adjust the size based on your maximum message size
    Monitor.readStringUntil('\n').toCharArray(message_send, sizeof(message_send));

    if (String(message_send).startsWith("LoRa:")){
      ResponseStatus responce = writeToLoRa(String(message_send));
    }
    else if (String(message_send).startsWith("RF:")){
      writeIntoNRF24(String(message_send));     
    }
  }

  listenToLoRa();

}

void writeIntoNRF24(String inputString){
  nRF24.stopListening();
  char input[32];
  inputString.trim();  // Remove leading and trailing whitespaces, if needed
  inputString.toCharArray(input, sizeof(input));

  bool report = nRF24.write(input, sizeof(input));
  nRF24.startListening();   
}

void listenToNRF24(){
  if (nRF24.available()) { 
    readFromNRF24();
  }  
}

String readFromNRF24(){
  char input[32];
  nRF24.read(&input, sizeof(input)); 
  Monitor.print("Received: "); 
  Monitor.println(input);

  String* splitStrings;
  int splitCount = splitString(String(input), '|', splitStrings);
  
  if (splitStrings[1].equals("OPENED") || splitStrings[1].equals("CLOSED")){
    webSocket.sendTXT(0, input);
    String key = "gate" + String(splitStrings[0].c_str());
    preferences.putBool(key.c_str(), String("OPENED") == splitStrings[1]);
  }
  
  return String(input);   
}



void listenToLoRa(){
  if (LoRa.available()>1) {
    String input = readFromLoRa();
    input.trim();

    Monitor.println(input);

    int firstPipeIndex = input.indexOf('|');
    int secondPipeIndex = input.indexOf('|', firstPipeIndex + 1);

    if (firstPipeIndex != -1 && secondPipeIndex != -1 && input.length() > 16) {
      // Two occurrences of "|" found in the string
      webSocket.sendTXT(0, input);
    }    
  }  
}

String readFromLoRa(){
  ResponseContainer rc = LoRa.receiveMessage();
  if (rc.status.code!=1){
      rc.status.getResponseDescription();
  }else{  
      return rc.data;
  }  
}

ResponseStatus writeToLoRa(String input){
  ResponseStatus rs = LoRa.sendMessage(input);
  return rs;
}


void clearInComingBuffer(HardwareSerial& serialObject) {
  while (serialObject.available()) {
    serialObject.read();  
  }
}

void clearOutComingBuffer(HardwareSerial& serialObject) {
  serialObject.flush();    
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

      break;
    // For anything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
      Monitor.println("Error with WebSocket");
      break;
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
    break; 
  }
}


void updateItems(String input){
  
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);

  if (splitStrings[0].equals(TIME_UPDATE_COMMAND)){
    Monitor.println("Old Time: " + String(cycle_time) + " || New Time: " + splitStrings[1]);
    cycle_time = splitStrings[1].toInt();
    preferences.putInt("time", cycle_time); 
    ResponseStatus responce = writeToLoRa(input);
  } 
  
  if (splitStrings[0].equals(GPS_ENABLE_COMMAND)){ ResponseStatus responce = writeToLoRa(input);; }
  if (splitStrings[0].equals(GPS_DISABLE_COMMAND)){ ResponseStatus responce = writeToLoRa(input);; }

    
  if (splitStrings[0].equals(GATE_CLOSE_COMMAND)){
    Monitor.println(input);
    nRF24.stopListening();  //не слушаем радиоэфир, мы передатчик
    nRF24.openWritingPipe(address[1]);   //мы - труба 0, открываем канал для передачи данных

    writeIntoNRF24(input);
  }
  if (splitStrings[0].equals(GATE_OPEN_COMMAND)){
    Monitor.println(input);
    nRF24.stopListening();  //не слушаем радиоэфир, мы передатчик
    nRF24.openWritingPipe(address[1]);   //мы - труба 0, открываем канал для передачи данных

    writeIntoNRF24(input);
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
