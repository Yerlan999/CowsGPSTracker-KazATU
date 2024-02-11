#define SMS_TARGET  "+77089194616"

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

void updateSerial();




void setup() {
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
  updateSerial();
  delay(200);
  SIM800.println("AT+CMGF=1"); // Конфигурация режима обработки текста
  updateSerial();
  delay(200);
  SIM800.println("AT+CNMI=2,2,0,0,0"); // Назначение очереди обработки входящих сообщении (СМС)
  updateSerial();
}

char smsBuffer[250]; // Размер буфера для хранения текста сообщении


void loop() {
    
    updateSerial(); // Чтение входящих данных

    if(Monitor.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
      String input = Monitor.readStringUntil('\n');
      LoRa.println(input);
      LoRa.flush();     
    }
  
    if(LoRa.available() > 0){
      String input = LoRa.readStringUntil('\n');
      Monitor.println(input);    
    }
    delay(20);
}




void updateSerial() {

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
      if (SMS_content == "GET GPS"){
        LoRa.println("GET GPS");
      }
      
      if (SMS_content == "Turn on") {
        Monitor.println("The pump has been turned on");        
      }
    }
    }
}

// Отправка сообщения
void sendSMSmessage(){
  if (modem.sendSMS(SMS_TARGET, "Message from ESP32")) {Monitor.println("Message from ESP32");}
  else {Monitor.println("SMS failed to send");}
  delay(1000); 
}
