#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

#define Monitor Serial

// Replace with your network credentials (STATION)
const char* ssid = "Le petit dejeuner 2";
const char* password = "DoesGodReallyExist404";

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
  // Initialize Monitor Monitor
  Monitor.begin(115200);

  Monitor.println();
  Monitor.print("Server MAC Address:  ");
  Monitor.println(WiFi.macAddress());

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Monitor.println("Setting as a Wi-Fi Station..");
  }

  Monitor.print("Server SOFT AP MAC Address:  ");
  Monitor.println(WiFi.softAPmacAddress());

  chan = WiFi.channel();
  Monitor.print("Station IP Address: ");
  Monitor.println(WiFi.localIP());
  Monitor.print("Wi-Fi Channel: ");
  Monitor.println(WiFi.channel());

  initESP_NOW();

}

void loop() {

  if (Monitor.available()) {
    String userMessage = Monitor.readString();
    userMessage.trim();
    readDataToSend();

    uint8_t byteArray[sizeof(outgoingMessage)];
    structToByteArray(outgoingMessage, byteArray);

    esp_err_t result = esp_now_send(NULL, byteArray, sizeof(outgoingMessage));
  
  }
  
}