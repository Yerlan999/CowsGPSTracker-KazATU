#include <string.h>
#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h> 
#include <ArduinoJson.h>

#define SMS_TARGET  "+7"

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER   1024  // RX буфер = 1Kb

#include <HardwareSerial.h>
#include <Wire.h>
#include <TinyGsmClient.h>

#define M0 12       
#define M1 14

// TTGO T-Call распиновка
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

// Последовательный цифровой порт для вывода текста на экран
#define Monitor Serial
// Последовательный цифровой порт для общения с Модулем SIM800H
#define SIM800  Serial2

String SMS_stream;
String SMS_content;

// Последовательный цифровой порт для дебаггинга?
#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SIM800, Monitor); // Оба порта
TinyGsm modem(debugger);
#else
TinyGsm modem(SIM800); // Только один порт (канал устройство-модуль SIM)
#endif

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

HardwareSerial LoRa(1); // use UART1


bool setPowerBoostKeepOn(int en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en) {
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    Wire.write(0x35); // 0x37 is default reg value
  }
  return Wire.endTransmission() == 0;
}

void listenSIM800();


String GPS_ENABLE_COMMAND = "START GPS";
String GPS_DISABLE_COMMAND = "STOP GPS";
String TIME_UPDATE_COMMAND = "TIME_UPDATE";

unsigned long lastTime = 0;
unsigned long cycle_time = 30000;    // КАЖДЫЕ N секунд

const char* ssid = "";
const char* password = "";
const char *host = "192.168.0.12";
const int port = 8000;

void setup() {
  
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Monitor.println("Connecting to WiFi..");
  }
 
  Monitor.println("Connected to the WiFi network");


  // Serial2.begin(9600);   // lora E32 gắn với cổng TX2 RX2 trên board ESP32
  Monitor.begin(9600);
  LoRa.begin(9600, SERIAL_8N1, 25, 33); 

  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal


  // Keep power when running from battery
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  Monitor.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SIM800.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  Monitor.println("Initializing modem...");
  // modem.restart();
  modem.init();


  SIM800.println("AT"); // Рукопожатие устройства ESP32 с модулем SIM800
  listenSIM800();
  delay(200);
  SIM800.println("AT+CMGF=1"); // Конфигурация режима обработки текста
  listenSIM800();
  delay(200);
  SIM800.println("AT+CNMI=2,2,0,0,0"); // Назначение очереди обработки входящих сообщении (СМС)
  listenSIM800();
}

char smsBuffer[250]; // Размер буфера для хранения текста сообщении


void loop() {
    
  listenSIM800(); // Чтение входящих данных

  if(Monitor.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
    String input = Monitor.readStringUntil('\n');
    LoRa.println(input);
  }

  if(LoRa.available() > 0){
    String input = LoRa.readStringUntil('\n');
    Monitor.println(input);    
  }

  if ((millis() - lastTime) > cycle_time) {
    
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
      // sendHttpPostRequest();
    }else{
      Monitor.println("Error in WiFi connection");   
    }
  lastTime = millis();
  }

  delay(20);
}




void listenSIM800() {

  while (SIM800.available()) {
    SMS_stream = SIM800.readStringUntil('\n');
    if (SMS_stream.startsWith("+CMT")) {
      Monitor.println("Stream: " + SMS_stream);
      
      String sender = SMS_stream.substring(7, 19);
      int stream_len = SMS_stream.length();
      String datetime = SMS_stream.substring(stream_len-22, stream_len-2);

      Monitor.println("Sender: " + sender);
      Monitor.println("DateTime: " + datetime);

      SMS_content = SIM800.readStringUntil('\n');
      SMS_content.trim();
      Monitor.println("Content: " + SMS_content);
      
      updateTime(SMS_content);
      
            
      LoRa.println(SMS_content);

    }
    }
}


void updateTime(String input){
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);

  if (splitStrings[0].equals(TIME_UPDATE_COMMAND)){
    Monitor.println("Old Time: " + String(cycle_time) + " || New Time: " + splitStrings[1]);
    cycle_time = splitStrings[1].toInt();  
  }      
  delete[] splitStrings;  
}



// Отправка сообщения
void sendSMSmessage(){
  if (modem.sendSMS(SMS_TARGET, "Message from ESP32")) {Monitor.println("Message from ESP32");}
  else {Monitor.println("SMS failed to send");}
  delay(1000); 
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


void sendHttpPostRequest() {
  HTTPClient http;

  // Create an array of dictionaries
  DynamicJsonDocument jsonDocument(2048);  // Adjust the size based on your data
  JsonArray jsonArray = jsonDocument.to<JsonArray>();

  // Example dictionaries
  JsonObject dict1 = jsonArray.createNestedObject();
  dict1["latitude"] = 54.214284;
  dict1["longitude"] = 69.514049;
  dict1["index"] = "1";

  JsonObject dict2 = jsonArray.createNestedObject();
  dict2["latitude"] = 54.214440;
  dict2["longitude"] = 69.511097;
  dict2["index"] = "2";

  JsonObject dict3 = jsonArray.createNestedObject();
  dict3["latitude"] = 54.213070;
  dict3["longitude"] = 69.511260;
  dict3["index"] = "3";

  // Add more dictionaries as needed...

  // Serialize the JSON object to a string
  String postData;
  serializeJson(jsonDocument, postData);





  // Make the POST request
  http.begin("http://" + String(host) + ":" + String(port) + "/ajax/");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    Serial.println(httpResponseCode);
  } else {
    Serial.println("Error on sending POST");
  }

  http.end();
}