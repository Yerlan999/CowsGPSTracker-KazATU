#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля

#define Monitor Serial

RF24 radio(4, 5); // "создать" модуль на пинах 9 и 10 Для Уно

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

void setup() {
  Monitor.begin(9600); //открываем порт для связи с ПК

  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
}

void loop() {
    radio.openReadingPipe(0,address[1]);      //хотим слушать трубу 0          
    radio.startListening();  //начинаем слушать эфир, мы приёмный модуль

    if( radio.available()){    // слушаем эфир со всех труб
      char message_from[32];
      radio.read( &message_from, sizeof(message_from) );         // чиатем входящий сигнал
      Monitor.print("Recieved: "); Monitor.println(message_from);
   }

  if (Monitor.available()){
    
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
    radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных

    char message_send[32]; // Adjust the size based on your maximum message size
    Monitor.readStringUntil('\n').toCharArray(message_send, sizeof(message_send));
    radio.write(&message_send, sizeof(message_send));  
  }
}