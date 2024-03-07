#include <WiFi.h>
#include <esp_wifi.h>
#include <WebSocketsServer.h>
#include <esp_now.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include "Arduino.h"
#include "LoRa_E32.h"
#include <map>
#include <ArduinoJson.h>

#define Monitor Serial

LoRa_E32 LoRa(&Serial2, 15, 22, 21);  //  Serial Pins,  AUX, M0, M1
WebSocketsServer webSocket = WebSocketsServer(80);
std::map<String, uint8_t*> addressDictionary;
Preferences preferences;

uint8_t Gate1Address[] = {0xA0, 0xA3, 0xB3, 0x2C, 0x0E, 0x50};
uint8_t Gate2Address[] = {0x24, 0x6F, 0x28, 0xB2, 0x27, 0x64};

String GPS_ENABLE_COMMAND = "START GPS";
String GPS_DISABLE_COMMAND = "STOP GPS";
String TIME_UPDATE_COMMAND = "TIME_UPDATE";

String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

String ssid;
String password;

unsigned long cycle_time;  // КАЖДЫЕ N секунд
esp_now_peer_info_t slave;
int chan; 

enum MessageType {PAIRING, DATA,};
MessageType messageType;

int counter = 0;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  
  uint8_t msgType;
  uint8_t id;
  char command[32];

} struct_message;

typedef struct struct_pairing {       // new structure for pairing
    uint8_t msgType;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
} struct_pairing;

struct_message incomingMessage;
struct_message outgoingMessage;
struct_pairing pairingData;

// Function to set the command field from a string
void setCommand(struct_message& message, const char* command) {
  strncpy(message.command, command, sizeof(message.command) - 1);
  message.command[sizeof(message.command) - 1] = '\0';  // Ensure null-termination
}

// Function to convert struct_message to a byte array
void structToByteArray(const struct_message& input, uint8_t* output) {
  memcpy(output, &input, sizeof(input));
}

// Function to convert byte array to struct_message
void byteArrayToStruct(const uint8_t* input, struct_message& output) {
  memcpy(&output, input, sizeof(output));
}

void readDataToSend() {
  outgoingMessage.msgType = DATA;
  outgoingMessage.id = 0;
  setCommand(outgoingMessage, "Command from Server");
}


// ---------------------------- esp_ now -------------------------
void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Monitor.print(macStr);
}

bool addPeer(const uint8_t *peer_addr) {      // add pairing
  memset(&slave, 0, sizeof(slave));
  const esp_now_peer_info_t *peer = &slave;
  memcpy(slave.peer_addr, peer_addr, 6);
  
  slave.channel = chan; // pick a channel
  slave.encrypt = 0; // no encryption
  // check if the peer exists
  bool exists = esp_now_is_peer_exist(slave.peer_addr);
  if (exists) {
    // Slave already paired.
    Monitor.println("Already Paired");
    return true;
  }
  else {
    esp_err_t addStatus = esp_now_add_peer(peer);
    if (addStatus == ESP_OK) {
      // Pair success
      Monitor.println("Pair success");
      return true;
    }
    else 
    {
      Monitor.println("Pair failed");
      return false;
    }
  }
} 

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Monitor.print("Last Packet Send Status: ");
  Monitor.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success to " : "Delivery Fail to ");
  printMAC(mac_addr);
  Monitor.println();
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  Monitor.print(len);
  Monitor.print(" bytes of data received from : ");
  printMAC(mac_addr);
  Monitor.println();
  StaticJsonDocument<1000> root;
  String payload;
  uint8_t type = incomingData[0];       // first message byte is the type of message 
  switch (type) {
  case DATA :
    byteArrayToStruct(incomingData, incomingMessage);  // the message is data type
    // memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
    // create a JSON document with received data and send it by event to the web page
    root["id"] = incomingMessage.id;
    root["command"] = incomingMessage.command;
    serializeJson(root, payload);
    Monitor.print("event send :");
    serializeJson(root, Monitor);
    Monitor.println();
    break;
  
  case PAIRING:                            // the message is a pairing request 
    memcpy(&pairingData, incomingData, sizeof(pairingData));
    Monitor.println(pairingData.msgType);
    Monitor.println(pairingData.id);
    Monitor.print("Pairing request from: ");
    printMAC(mac_addr);
    Monitor.println();
    Monitor.println(pairingData.channel);
    if (pairingData.id > 0) {     // do not replay to server itself
      if (pairingData.msgType == PAIRING) { 
        pairingData.id = 0;       // 0 is server
        // Server is in AP_STA mode: peers need to send data to server soft AP MAC address 
        WiFi.softAPmacAddress(pairingData.macAddr);   
        pairingData.channel = chan;
        Monitor.println("send response");
        esp_err_t result = esp_now_send(mac_addr, (uint8_t *) &pairingData, sizeof(pairingData));
        addPeer(mac_addr);
      }  
    }  
    break; 
  }
}

void initESP_NOW(){
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Monitor.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
}

void setup() {

  addressDictionary["1"] = Gate1Address;
  addressDictionary["2"] = Gate2Address;


  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");

  cycle_time = preferences.getInt("time");

  Monitor.begin(9600);  //открываем порт для связи с ПК

  LoRa.begin();

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Monitor.println("Connecting to WiFi..");
  }

  Monitor.println("Connected to the WiFi network");
  Monitor.println(WiFi.localIP());

  Monitor.print("Server SOFT AP MAC Address:  ");
  Monitor.println(WiFi.softAPmacAddress());

  chan = WiFi.channel();
  Monitor.print("Station IP Address: ");
  Monitor.println(WiFi.localIP());
  Monitor.print("Wi-Fi Channel: ");
  Monitor.println(WiFi.channel());

  initESP_NOW();
  
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

}


void loop() {

  webSocket.loop();

  if (Monitor.available()) {

    String userMessage = Monitor.readStringUntil('\n');
    userMessage.trim();

    if (userMessage.startsWith("LoRa:")) {
      ResponseStatus responce = writeToLoRa(userMessage.substring(5));
    }
    if (userMessage.startsWith("GATE1:")) { 

      readDataToSend();
      uint8_t byteArray[sizeof(outgoingMessage)];
      structToByteArray(outgoingMessage, byteArray);
      esp_err_t result = esp_now_send(NULL, byteArray, sizeof(outgoingMessage));

    }
    if (userMessage.startsWith("GATE2:")) { 
      
      readDataToSend();
      uint8_t byteArray[sizeof(outgoingMessage)];
      structToByteArray(outgoingMessage, byteArray);
      esp_err_t result = esp_now_send(NULL, byteArray, sizeof(outgoingMessage));

    }
  }

  listenToLoRa();
}


void listenToLoRa() {
  if (LoRa.available() > 1) {
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

String readFromLoRa() {
  ResponseContainer rc = LoRa.receiveMessage();
  if (rc.status.code != 1) {
    rc.status.getResponseDescription();
  } else {
    return rc.data;
  }
}

ResponseStatus writeToLoRa(String input) {
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
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  //Figure out the type of Websocket Event

  char receivedMessage[length + 1];

  switch (type) {
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


void updateItems(String input) {

  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);

  if (splitStrings[0].equals(TIME_UPDATE_COMMAND)) {
    Monitor.println("Old Time: " + String(cycle_time) + " || New Time: " + splitStrings[1]);
    cycle_time = splitStrings[1].toInt();
    preferences.putInt("time", cycle_time);
    ResponseStatus responce = writeToLoRa(input);
  }

  if (splitStrings[0].equals(GPS_ENABLE_COMMAND)) { ResponseStatus responce = writeToLoRa(input); }
  if (splitStrings[0].equals(GPS_DISABLE_COMMAND)) { ResponseStatus responce = writeToLoRa(input); }


  String key = "gate" + splitStrings[1];

  if (splitStrings[0].equals(GATE_CLOSE_COMMAND) && preferences.getBool(key.c_str())) {
    Monitor.println(input);
    // !!! sendMessageToSlave(addressDictionary[splitStrings[1]], input);
  }
  if (splitStrings[0].equals(GATE_OPEN_COMMAND) && !preferences.getBool(key.c_str())) {
    Monitor.println(input);
    // !!! sendMessageToSlave(addressDictionary[splitStrings[1]], input);
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