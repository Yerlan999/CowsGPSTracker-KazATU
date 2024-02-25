#include <WiFi.h> 
#include <WebSocketsServer.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <Preferences.h>
#include <HardwareSerial.h>

bool enable_SIM800L = false;

#define Monitor Serial
#define SIM800L Serial2

#define M0 22       
#define M1 21

HardwareSerial LoRa(1); // use UART1
RF24 radio(4, 5); // "создать" модуль на пинах 9 и 10 Для Уно
WebSocketsServer webSocket= WebSocketsServer(80);
Preferences preferences;


byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб

String GPS_ENABLE_COMMAND = "START GPS";
String GPS_DISABLE_COMMAND = "STOP GPS";
String TIME_UPDATE_COMMAND = "TIME_UPDATE";

String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

String ssid;  
String password;
String SMS_stream;
String SMS_content;

unsigned long cycle_time;    // КАЖДЫЕ N секунд

void setup(){
  
  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", ""); 
  password = preferences.getString("password", "");

  cycle_time = preferences.getInt("time");

  Monitor.begin(9600); //открываем порт для связи с ПК
  
  LoRa.begin(9600, SERIAL_8N1, 15, 2);

  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0,15);     //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах
  
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  
  radio.powerUp(); //начать работу



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
  digitalWrite(M0, LOW);       
  digitalWrite(M1, LOW);       
  
  if (enable_SIM800L){
    //Begin serial communication with Arduino and SIM800L
    SIM800L.begin(9600);

    delay(1000);

    // Handshake test
    SIM800L.println("AT");
    listenSIM800L();

    // Configuring TEXT mode
    SIM800L.println("AT+CMGF=1");
    listenSIM800L();

    SIM800L.println("AT+CNMI=2,2,0,0,0"); // Назначение очереди обработки входящих сообщении (СМС)
    listenSIM800L();
  }
}

void loop() {

  if (enable_SIM800L){
    listenSIM800L();
  }
  
  webSocket.loop();

  radio.openReadingPipe(0,address[0]);      //хотим слушать трубу 0          
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль

  if( radio.available()){    // слушаем эфир со всех труб
    char message_from[32];
    radio.read( &message_from, sizeof(message_from) );         // чиатем входящий сигнал
    Monitor.print("Recieved: "); Monitor.println(message_from);
  }

  if (Monitor.available()){
    
    char message_send[32]; // Adjust the size based on your maximum message size
    Monitor.readStringUntil('\n').toCharArray(message_send, sizeof(message_send));

    if (String(message_send).startsWith("LoRa:")){
      LoRa.println(message_send);
    }
    else if (String(message_send).startsWith("RF:")){
      radio.stopListening();  //не слушаем радиоэфир, мы передатчик
      radio.openWritingPipe(address[1]);   //мы - труба 0, открываем канал для передачи данных

      radio.write(&message_send, sizeof(message_send));      
    }
    else if(String(message_send).startsWith("SMS:")){
      sendSMS("+77089194616", message_send);
    }
  }

  if(LoRa.available() > 0){
    String input = LoRa.readStringUntil('\n');
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

void sendSMS(const char* phoneNumber, const char* messageContent) {
  SIM800L.println("AT"); // Once the handshake test is successful, it will back to OK
  listenSIM800L();

  SIM800L.println("AT+CMGF=1"); // Configuring TEXT mode
  listenSIM800L();

  SIM800L.print("AT+CMGS=\"");
  SIM800L.print(phoneNumber);
  SIM800L.println("\""); // Change ZZ with country code and xxxxxxxxxxx with phone number to SMS
  listenSIM800L();

  SIM800L.print(messageContent); // Text content
  listenSIM800L();
  
  SIM800L.write(26);  
}

void listenSIM800L(){
  delay(500);

  while(SIM800L.available()) {
    SMS_stream = SIM800L.readStringUntil('\n');
    if (SMS_stream.startsWith("+CMT")) {
      Monitor.println("Stream: " + SMS_stream);
      
      String sender = SMS_stream.substring(7, 19);
      int stream_len = SMS_stream.length();
      String datetime = SMS_stream.substring(stream_len-22, stream_len-2);

      Monitor.println("Sender: " + sender);
      Monitor.println("DateTime: " + datetime);

      SMS_content = SIM800L.readStringUntil('\n');
      SMS_content.trim();
      Monitor.println("Content: " + SMS_content);

    }
  }
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
    LoRa.println(input); 
  } 
  
  if (splitStrings[0].equals(GPS_ENABLE_COMMAND)){ LoRa.println(input); }
  if (splitStrings[0].equals(GPS_DISABLE_COMMAND)){ LoRa.println(input); }

    
  if (splitStrings[0].equals(GATE_CLOSE_COMMAND)){
    Monitor.println(input);
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
    radio.openWritingPipe(address[1]);   //мы - труба 0, открываем канал для передачи данных

    radio.write(&input, sizeof(input));
  }
  if (splitStrings[0].equals(GATE_OPEN_COMMAND)){
    Monitor.println(input);
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
    radio.openWritingPipe(address[1]);   //мы - труба 0, открываем канал для передачи данных

    radio.write(&input, sizeof(input));
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
