#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define Monitor Serial
#define SIM800L Serial2

RF24 radio(4, 5); // "создать" модуль на пинах 9 и 10 Для Уно

byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб

String SMS_stream;
String SMS_content;

void setup(){
  Monitor.begin(9600); //открываем порт для связи с ПК
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


  //Begin serial communication with Arduino and SIM800L
  SIM800L.begin(9600);

  Monitor.println("Initializing...");
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

void loop() {

  listenSIM800L();

  radio.openReadingPipe(0,address[0]);      //хотим слушать трубу 0          
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль

  if( radio.available()){    // слушаем эфир со всех труб
    char message_from[32];
    radio.read( &message_from, sizeof(message_from) );         // чиатем входящий сигнал
    Monitor.print("Recieved: "); Monitor.println(message_from);
  }

  if (Monitor.available()){
    
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
    radio.openWritingPipe(address[1]);   //мы - труба 0, открываем канал для передачи данных

    char message_send[32]; // Adjust the size based on your maximum message size
    Monitor.readStringUntil('\n').toCharArray(message_send, sizeof(message_send));

    sendSMS("+77089194616", message_send);
    radio.write(&message_send, sizeof(message_send));  
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