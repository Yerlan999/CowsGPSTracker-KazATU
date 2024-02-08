// COLLAR MODULE ESP32

#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля
#include <LiquidCrystal_I2C.h>

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);  

RF24 radio(4, 5); // "создать" модуль на пинах 9 и 10 Для Уно

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб
String data_send; 

void setup() {
  lcd.init();
  lcd.backlight();

  Serial.begin(9600); //открываем порт для связи с ПК

  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0,15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  // radio.openReadingPipe(1,address[0]);      //хотим слушать трубу 0

  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
}

void loop() {

  byte pipeNo; String data_got;                          
  while(radio.available(&pipeNo)){    // слушаем эфир со всех труб
    radio.read( &data_got, sizeof(data_got) );         // чиатем входящий сигнал
    displayInfo(data_got, data_send);
    Serial.print("Recieved: "); Serial.println(data_got);
  }
  
  if (Serial.available()){
    data_send = Serial.readStringUntil('\n');
    Serial.print("Sent: "); Serial.println(data_send);
    displayInfo(data_got, data_send);
    radio.write(&data_send, sizeof(data_send));
  }
}

void displayInfo(String data_got, String data_send){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Send: " + String(data_send));
  lcd.setCursor(0, 1);
  lcd.print("Recieved: " + String(data_got));  
}
