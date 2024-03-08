// GATE #1

#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <EEPROM.h>

#define Monitor Serial
// Set your Board and Server ID 
#define BOARD_ID 1
#define MAX_CHANNEL 11  // for North America // 13 in Europe


const int DIR = 23;
const int STEP = 21;
const int  steps_per_rev = 200;
const int close_contact = 5; // PINK
const int open_contact = 15; // GREEN

const int motor_id = 1;

String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

uint8_t serverAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//Structure to send data
//Must match the receiver structure
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

//Create 2 struct_message 
struct_message outgoingMessage;  // data to send
struct_message incomingMessage;  // data received
struct_pairing pairingData;

enum PairingStatus {NOT_PAIRED, PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED,};
PairingStatus pairingStatus = NOT_PAIRED;

enum MessageType {PAIRING, DATA,};
MessageType messageType;

#ifdef SAVE_CHANNEL
  int lastChannel;
#endif  
int channel = 1;
 
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings
unsigned long start;                // used to measure Pairing time
unsigned int readingId = 0;   


void addPeer(const uint8_t * mac_addr, uint8_t chan){
  esp_now_peer_info_t peer;
  ESP_ERROR_CHECK(esp_wifi_set_channel(chan ,WIFI_SECOND_CHAN_NONE));
  esp_now_del_peer(mac_addr);
  memset(&peer, 0, sizeof(esp_now_peer_info_t));
  peer.channel = chan;
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac_addr, sizeof(uint8_t[6]));
  if (esp_now_add_peer(&peer) != ESP_OK){
    Monitor.println("Failed to add peer");
    return;
  }
  memcpy(serverAddress, mac_addr, sizeof(uint8_t[6]));
}

void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Monitor.print(macStr);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Monitor.print("\r\nLast Packet Send Status:\t");
  Monitor.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  Monitor.print("Packet received from: ");
  printMAC(mac_addr);
  Monitor.println();
  Monitor.print("data size = ");
  Monitor.println(sizeof(incomingData));
  uint8_t type = incomingData[0];
  switch (type) {
  case DATA :      // we received data from server
    byteArrayToStruct(incomingData, incomingMessage);  // the message is data type
    // memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
    Monitor.print("ID  = ");
    Monitor.println(incomingMessage.id);
    Monitor.print("Command = ");
    Monitor.println(incomingMessage.command);
    
    updateItems(String(incomingMessage.command));
    
    break;

  case PAIRING:    // we received pairing data from server
    memcpy(&pairingData, incomingData, sizeof(pairingData));
    if (pairingData.id == 0) {              // the message comes from server
      printMAC(mac_addr);
      Monitor.print("Pairing done for ");
      printMAC(pairingData.macAddr);
      Monitor.print(" on channel " );
      Monitor.print(pairingData.channel);    // channel used by the server
      Monitor.print(" in ");
      Monitor.print(millis()-start);
      Monitor.println("ms");
      addPeer(pairingData.macAddr, pairingData.channel); // add the server  to the peer list 
      #ifdef SAVE_CHANNEL
        lastChannel = pairingData.channel;
        EEPROM.write(0, pairingData.channel);
        EEPROM.commit();
      #endif  
      pairingStatus = PAIR_PAIRED;             // set the pairing status
    }
    break;
  }  
}

PairingStatus autoPairing(){
  switch(pairingStatus) {
    case PAIR_REQUEST:
    Monitor.print("Pairing request on channel "  );
    Monitor.println(channel);

    // set WiFi channel   
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel,  WIFI_SECOND_CHAN_NONE));
    if (esp_now_init() != ESP_OK) {
      Monitor.println("Error initializing ESP-NOW");
    }

    // set callback routines
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
  
    // set pairing data to send to the server
    pairingData.msgType = PAIRING;
    pairingData.id = BOARD_ID;     
    pairingData.channel = channel;

    // add peer and send request
    addPeer(serverAddress, channel);
    esp_now_send(serverAddress, (uint8_t *) &pairingData, sizeof(pairingData));
    previousMillis = millis();
    pairingStatus = PAIR_REQUESTED;
    break;

    case PAIR_REQUESTED:
    // time out to allow receiving response from server
    currentMillis = millis();
    if(currentMillis - previousMillis > 250) {
      previousMillis = currentMillis;
      // time out expired,  try next channel
      channel ++;
      if (channel > MAX_CHANNEL){
         channel = 1;
      }   
      pairingStatus = PAIR_REQUEST;
    }
    break;

    case PAIR_PAIRED:
      // nothing to do here 
    break;
  }
  return pairingStatus;
}


// Function to convert struct_message to a byte array
void structToByteArray(const struct_message& input, uint8_t* output) {
  memcpy(output, &input, sizeof(input));
}

// Function to convert byte array to struct_message
void byteArrayToStruct(const uint8_t* input, struct_message& output) {
  memcpy(&output, input, sizeof(output));
}

// Function to set the command field from a string
void setCommand(struct_message& message, const char* command) {
  strncpy(message.command, command, sizeof(message.command) - 1);
  message.command[sizeof(message.command) - 1] = '\0';  // Ensure null-termination
}

void setup() {
  Monitor.begin(9600); //открываем порт для связи с ПК

  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(close_contact, INPUT_PULLUP);
  pinMode(open_contact, INPUT_PULLUP);
  Monitor.println();
  Monitor.print("Client Board MAC Address:  ");
  Monitor.println(WiFi.macAddress());
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  start = millis();

  #ifdef SAVE_CHANNEL 
    EEPROM.begin(10);
    lastChannel = EEPROM.read(0);
    Monitor.println(lastChannel);
    if (lastChannel >= 1 && lastChannel <= MAX_CHANNEL) {
      channel = lastChannel; 
    }
    Monitor.println(channel);
  #endif  
  pairingStatus = PAIR_REQUEST;
}


void SendToServer(const char* response){
  outgoingMessage.msgType = DATA;
  outgoingMessage.id = BOARD_ID;
  setCommand(outgoingMessage, response);

  uint8_t byteArray[sizeof(outgoingMessage)];
  structToByteArray(outgoingMessage, byteArray);

  esp_err_t result = esp_now_send(serverAddress, byteArray, sizeof(outgoingMessage));
}


void loop() {
  
  if (autoPairing() == PAIR_PAIRED) {

    if (Monitor.available()) {
      
      String userMessage = Monitor.readString();
      userMessage.trim();
      SendToServer(userMessage.c_str());
    
    }
      
  }
}

void updateItems(String input){
  
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);
  Monitor.println(input);
    
  if (splitStrings[0].equals(GATE_CLOSE_COMMAND)){
    moveStepper(0, splitStrings[1].toInt());
  }
  if (splitStrings[0].equals(GATE_OPEN_COMMAND)){
    moveStepper(1, splitStrings[1].toInt());
  }

  delete[] splitStrings;  
}

bool readLimitSwitch(String mode) {
  int totalSwitchState = 0;
  for (int i = 0; i < 20; i++) {
    if (mode == "CLOSED?"){ totalSwitchState += digitalRead(close_contact); };
    if (mode == "OPENED?"){ totalSwitchState += digitalRead(open_contact); }; 
    delay(10); // Add a small delay between readings
  }
  return totalSwitchState == 0;
}


void moveStepper(bool direction, int gate_ID){
  int current_direction = direction;

  digitalWrite(DIR, direction);
  if (gate_ID == motor_id){
    Monitor.println("Moving Stepper #" + String(motor_id));
    
    // SendToServer(("Moving Stepper #" + String(motor_id)).c_str());

    for(;;){
      digitalWrite(STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(STEP, LOW);
      delayMicroseconds(500);

      if (current_direction==0 && !digitalRead(close_contact)){
      
        String formattedMessage = createMessage(gate_ID, "CLOSED");      
        SendToServer(formattedMessage.c_str());
            
        break;
      }

      if (current_direction==1 && !digitalRead(open_contact)){
        
        String formattedMessage = createMessage(gate_ID, "OPENED");  
        SendToServer(formattedMessage.c_str());

        break;
      }

    }
    delay(1000);    
  }
  else{
    Monitor.println("Missing Stepper Motor");
  }  
}

String createMessage(int gate_ID, const char* status) {
  // Create a String and convert it to a C-style string
  String messageString = String(gate_ID) + "|" + status;
  const char* message_cstr = messageString.c_str();

  // Copy the C-style string to a char array
  char message_send[32];
  strncpy(message_send, message_cstr, sizeof(message_send) - 1);
  message_send[sizeof(message_send) - 1] = '\0';  // Ensure null-termination

  // Return the formatted message
  return String(message_send);
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