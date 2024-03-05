#include <esp_now.h>
#include <WiFi.h>

// Replace with the MAC Addresses of the two slaves
uint8_t Gate1Address[] = {0xA0, 0xA3, 0xB3, 0x2C, 0x0E, 0x50};
uint8_t Gate2Address[] = {0x24, 0x6F, 0x28, 0xB2, 0x27, 0x64};

typedef struct struct_message {
    char message[50];
} struct_message;

struct_message outgoingMessage;
struct_message incomingMessage;

esp_now_peer_info_t peerInfoSlave1;
esp_now_peer_info_t peerInfoSlave2;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Serial.print("Received message from slave: ");
  Serial.println(incomingMessage.message);
}

void setup() {
  // Init Serial Monitor
  Serial.begin(9600);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register for a callback function that will be called when data is sent
  esp_now_register_send_cb(OnDataSent);

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  // Register peers (slaves)
  memcpy(peerInfoSlave1.peer_addr, Gate1Address, 6);
  peerInfoSlave1.channel = 0;
  peerInfoSlave1.encrypt = false;

  if (esp_now_add_peer(&peerInfoSlave1) != ESP_OK) {
    Serial.println("Failed to add slave1");
    return;
  }

  memcpy(peerInfoSlave2.peer_addr, Gate2Address, 6);
  peerInfoSlave2.channel = 0;
  peerInfoSlave2.encrypt = false;

  if (esp_now_add_peer(&peerInfoSlave2) != ESP_OK) {
    Serial.println("Failed to add slave2");
    return;
  }
}

void loop() {
  // Check if there is a message to send to slave1
  if (Serial.available()) {
    String userMessage = Serial.readString();
    userMessage.trim();
    
    if (userMessage.startsWith("GATE1:")){ sendMessageToSlave(Gate1Address, userMessage.substring(6)); }
    if (userMessage.startsWith("GATE2:")){ sendMessageToSlave(Gate2Address, userMessage.substring(6)); }

    delay(2000);
  }
}

void sendMessageToSlave(uint8_t* slaveAddress, String message) {
  strncpy(outgoingMessage.message, message.c_str(), sizeof(outgoingMessage.message));
  esp_err_t result = esp_now_send(slaveAddress, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage));

  if (result == ESP_OK) {
    Serial.println("Message sent with success");
  } else {
    Serial.println("Error sending message");
  }
}
