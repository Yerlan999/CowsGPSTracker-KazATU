// GATE #1

#include <esp_now.h>
#include <WiFi.h>

#define Monitor Serial

const int DIR = 23;
const int STEP = 21;
const int  steps_per_rev = 200;
const int close_contact = 5; // PINK
const int open_contact = 4; // GREEN

const int motor_id = 1;

String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

uint8_t HeadAddress[] = {0x78, 0xE3, 0x6D, 0x12, 0x00, 0x2C};

typedef struct struct_message {
    char message[50];
} struct_message;

struct_message outgoingMessage;
struct_message incomingMessage;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Monitor.print("\r\nLast Packet Send Status:\t");
  Monitor.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Monitor.print("Received message: ");
  Monitor.println(incomingMessage.message);
  updateItems(String(incomingMessage.message));
}

void setup() {
  Monitor.begin(9600); //открываем порт для связи с ПК

  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(close_contact, INPUT);
  pinMode(open_contact, INPUT);
  
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) { Monitor.println("Error initializing ESP-NOW"); return; }
  // Register for a callback function that will be called when data is sent
  esp_now_register_send_cb(OnDataSent);
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  // Register peer
  memcpy(peerInfo.peer_addr, HeadAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) { Monitor.println("Failed to add peer"); return; }
}

void loop() {
  
  if (Monitor.available()){
    String input = Monitor.readStringUntil('\n');
    sendToHeader(input);
  }
}

void sendToHeader(String userMessage){
  strncpy(outgoingMessage.message, userMessage.c_str(), sizeof(outgoingMessage.message));
  esp_err_t result = esp_now_send(HeadAddress, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage));
  if (result == ESP_OK) { Monitor.println("Sent with success"); } 
  else { Monitor.println("Error sending the data"); }  
}

void updateItems(String input){
  
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);
    
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

bool readLimitSwitch(String mode) {
  int totalSwitchState = 0;
  for (int i = 0; i < 20; i++) {
    if (mode == "CLOSED?"){ totalSwitchState += digitalRead(close_contact); };
    if (mode == "OPENED?"){ totalSwitchState += digitalRead(open_contact); }; 
    delay(10); // Add a small delay between readings
  }
  return totalSwitchState == 20;
}


void moveStepper(bool direction, int gate_ID){
  int current_direction = direction;

  digitalWrite(DIR, direction);
  if (gate_ID == motor_id){
    Monitor.println("Moving Stepper #" + String(motor_id));
    
    sendToHeader("Moving Stepper #" + String(motor_id));

    for(;;){
      digitalWrite(STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(STEP, LOW);
      delayMicroseconds(500);

      if (current_direction==0 && readLimitSwitch("CLOSED?")){
      
        String formattedMessage = createMessage(gate_ID, "CLOSED");      
        sendToHeader(formattedMessage);
            
        break;
      }

      if (current_direction==1 && readLimitSwitch("OPENED?")){
        
        String formattedMessage = createMessage(gate_ID, "OPENED");  
        sendToHeader(formattedMessage);

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

void clearInComingBuffer(HardwareSerial& serialObject) {
  while (serialObject.available()) {
    serialObject.read();  
  }
}

void clearOutComingBuffer(HardwareSerial& serialObject) {
  serialObject.flush();    
}