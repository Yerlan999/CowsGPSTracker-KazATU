// GATE #1

#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля

#define Monitor Serial

const int DIR = 32;
const int STEP = 33;
const int  steps_per_rev = 200;
const int close_contact = 25; // PINK
const int open_contact = 26; // GREEN

const int motor_id = 1;

String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

RF24 nRF24(4, 5); // CE, CSN

const byte address[2][6] = {"00001", "00002"};

const int listen_to = 1;
const int write_into = 0;

void setup() {
  Monitor.begin(9600); //открываем порт для связи с ПК

  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(close_contact, INPUT);
  pinMode(open_contact, INPUT);
  
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
}

void loop() {
  
  listenToNRF24();

  if (Monitor.available()){
    String input = Monitor.readStringUntil('\n');
    writeIntoNRF24(input);    
  }
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
  char message_from[32];
  nRF24.read( &message_from, sizeof(message_from) );         // чиатем входящий сигнал
  delay(100);
  Monitor.print("Recieved: "); Monitor.println(message_from);
  updateItems(String(message_from));
  return String(message_from);   
}

void clearInComingBuffer(HardwareSerial& serialObject) {
  while (serialObject.available()) {
    serialObject.read();  
  }
}

void clearOutComingBuffer(HardwareSerial& serialObject) {
  serialObject.flush();    
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
  for (int i = 0; i < 100; i++) {
    if (mode == "CLOSED?"){ totalSwitchState += digitalRead(close_contact); };
    if (mode == "OPENED?"){ totalSwitchState += digitalRead(open_contact); }; 
    delay(10); // Add a small delay between readings
  }
  return totalSwitchState == 100;
}

void moveStepper(bool direction, int gate_ID){
  int current_direction = direction;

  digitalWrite(DIR, direction);
  if (gate_ID == motor_id){
    Monitor.println("Moving Stepper #" + String(motor_id));

    for(;;){
      digitalWrite(STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(STEP, LOW);
      delayMicroseconds(500);

      if (current_direction==0 && readLimitSwitch("CLOSED?")){
      
        String formattedMessage = createMessage(gate_ID, "CLOSED");      
        writeIntoNRF24(String(formattedMessage));
            
        break;
      }

      if (current_direction==1 && readLimitSwitch("OPENED?")){
        
        String formattedMessage = createMessage(gate_ID, "OPENED");  
        writeIntoNRF24(String(formattedMessage));

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

