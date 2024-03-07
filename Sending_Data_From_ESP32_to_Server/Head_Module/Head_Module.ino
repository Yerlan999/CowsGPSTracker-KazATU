#include <WiFi.h>
#include <esp_wifi.h>
#include <WebSocketsServer.h>
#include <esp_now.h>
#include <Preferences.h>
#include <HardwareSerial.h>
#include "Arduino.h"
#include "LoRa_E32.h"
#include <map>

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

typedef struct struct_message {
  char message[50];
} struct_message;

struct_message outgoingMessage;
struct_message incomingMessage;

esp_now_peer_info_t peerInfoSlave1;
esp_now_peer_info_t peerInfoSlave2;

// Callback when data is sent
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  Monitor.print("\r\nLast Packet Send Status:\t");
  Monitor.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Monitor.print("Received message from slave: ");
  Monitor.println(incomingMessage.message);
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

  esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Monitor.println("Error initializing ESP-NOW");
    return;
  }

  // Register for a callback function that will be called when data is sent
  esp_now_register_send_cb(OnDataSent);
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  // Register peers (slaves)
  memcpy(peerInfoSlave1.peer_addr, addressDictionary["1"], 6);
  peerInfoSlave1.channel = 0;
  peerInfoSlave1.encrypt = false;

  if (esp_now_add_peer(&peerInfoSlave1) != ESP_OK) {
    Monitor.println("Failed to add slave1");
    return;
  }

  memcpy(peerInfoSlave2.peer_addr, addressDictionary["2"], 6);
  peerInfoSlave2.channel = 0;
  peerInfoSlave2.encrypt = false;

  if (esp_now_add_peer(&peerInfoSlave2) != ESP_OK) {
    Monitor.println("Failed to add slave2");
    return;
  }
  
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

    if (userMessage.startsWith("GATE1:")) { sendMessageToSlave(Gate1Address, userMessage.substring(6)); }
    if (userMessage.startsWith("GATE2:")) { sendMessageToSlave(Gate2Address, userMessage.substring(6)); }
  }

  listenToLoRa();
}


void sendMessageToSlave(uint8_t* slaveAddress, String message) {
  strncpy(outgoingMessage.message, message.c_str(), sizeof(outgoingMessage.message));
  esp_err_t result = esp_now_send(slaveAddress, (uint8_t*)&outgoingMessage, sizeof(outgoingMessage));

  if (result == ESP_OK) {
    Monitor.println("Message sent with success");
  } else {
    Monitor.println("Error sending message");
  }
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
    sendMessageToSlave(addressDictionary[splitStrings[1]], input);
  }
  if (splitStrings[0].equals(GATE_OPEN_COMMAND) && !preferences.getBool(key.c_str())) {
    Monitor.println(input);
    sendMessageToSlave(addressDictionary[splitStrings[1]], input);
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