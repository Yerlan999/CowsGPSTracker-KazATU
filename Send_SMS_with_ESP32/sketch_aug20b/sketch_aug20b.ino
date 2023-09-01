#define SMS_TARGET  "+77089194616"

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER   1024  // RX буфер = 1Kb

#include <Wire.h>
#include <TinyGsmClient.h>

// TTGO T-Call распиновка
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

// Последовательный цифровой порт для вывода текста на экран
#define SerialMon Serial
// Последовательный цифровой порт для общения с Модулем SIM800H
#define SerialAT  Serial1

String SMS_stream;
String SMS_content;

// Последовательный цифровой порт для дебаггинга?
#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon); // Оба порта
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT); // Только один порт (канал устройство-модуль SIM)
#endif

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

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
  Serial.begin(115200);
  // Keep power when running from battery
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  SerialMon.println("Initializing modem...");
  // modem.restart();
  modem.init();


  SerialAT.println("AT"); // Рукопожатие устройства ESP32 с модулем SIM800
  updateSerial();
  delay(200);
  SerialAT.println("AT+CMGF=1"); // Конфигурация режима обработки текста
  updateSerial();
  delay(200);
  SerialAT.println("AT+CNMI=2,2,0,0,0"); // Назначение очереди обработки входящих сообщении (СМС)
  updateSerial();

}


char smsBuffer[250]; // Размер буфера для хранения текста сообщении


void loop() {

  updateSerial(); // Чтение входящих данных

  // Отправка сообщения случайным образом
  int random_number = random(100);
  if (random_number == 1000) {
    if (modem.sendSMS(SMS_TARGET, "Message from ESP32")) {SerialMon.println("Message from ESP32");}
    else {SerialMon.println("SMS failed to send");}
  }
  delay(1000);
}

void updateSerial() {
  // while (SerialMon.available()) {
  //   SerialAT.write(SerialMon.read());
  //   }
  while (SerialAT.available()) {
    SMS_stream = SerialAT.readStringUntil('\n');
    if (SMS_stream.startsWith("+CMT")) {
      SerialMon.println("Check sender: " + SMS_stream);
      SMS_content = SerialAT.readStringUntil('\n');
      SerialMon.println("Check content: " + SMS_content);
    }
    // SerialMon.write(SerialAT.read());
    }
}
