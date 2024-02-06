// INTERMEDIATE MODULE ESP32 SIM800H

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <LiquidCrystal_I2C.h>

#define BUTTON_PIN 15 // GPIO21 pin connected to button

int lcdColumns = 20;
int lcdRows = 4;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  
RF24 radio(4, 5); // "создать" модуль на пинах 9 и 10 Для Уно

byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб
byte data_send;
int counter = 0;

void setup(){
  pinMode(BUTTON_PIN, INPUT);
  lcd.init();
  lcd.backlight();

  Serial.begin(9600); //открываем порт для связи с ПК
  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0,15);     //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openReadingPipe(1,address[0]);      //хотим слушать трубу 0
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  
  radio.powerUp(); //начать работу
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
}

void loop() {
    byte pipeNo, data_got;                          
    while(radio.available(&pipeNo)){    // слушаем эфир со всех труб
      radio.read( &data_got, sizeof(data_got) );         // чиатем входящий сигнал
      
      displayInfo(data_got, data_send);
      
      Serial.print("Recieved: "); Serial.println(data_got);
   }

  int buttonState = digitalRead(BUTTON_PIN);
  delay(250);
  if (buttonState){
    counter++;
    displayInfo(data_got, counter);
  }


}

void displayInfo(byte data_got, byte data_send){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Send: " + String(data_send));
  lcd.setCursor(0, 1);
  lcd.print("Recieved: " + String(data_got));  
}