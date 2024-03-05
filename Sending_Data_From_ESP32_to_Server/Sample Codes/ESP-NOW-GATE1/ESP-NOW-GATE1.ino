#include <esp_now.h>
#include <WiFi.h>

// Replace with the MAC Address of the other ESP32
uint8_t HeadAddress[] = {0x78, 0xE3, 0x6D, 0x12, 0x00, 0x2C};

typedef struct struct_message {
    char message[50];
} struct_message;

struct_message outgoingMessage;
struct_message incomingMessage;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Serial.print("Received message: ");
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

  // Register peer
  memcpy(peerInfo.peer_addr, HeadAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // Check if there is a message to send
  if (Serial.available()) {
    String userMessage = Serial.readString();
    userMessage.trim();

    // Set the message to send
    strncpy(outgoingMessage.message, userMessage.c_str(), sizeof(outgoingMessage.message));

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(HeadAddress, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage));

    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }

    // Wait for a moment before sending another message
    delay(2000);
  }
}
