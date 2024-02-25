#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля

#define Monitor Serial

const int DIR = 1;
const int STEP = 3;
const int  steps_per_rev = 200;
const int end_contact1 = 0;

String GATE_CLOSE_COMMAND = "CLOSE";
String GATE_OPEN_COMMAND = "OPEN";

RF24 radio(2, 4); // "создать" модуль на пинах 9 и 10 Для Уно

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

void setup() {
  Monitor.begin(9600); //открываем порт для связи с ПК

  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(end_contact1, INPUT);
  
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
      updateItems(String(message_from));
   }

  if (Monitor.available()){
    
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
    radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных

    char message_send[32]; // Adjust the size based on your maximum message size
    Monitor.readStringUntil('\n').toCharArray(message_send, sizeof(message_send));
    radio.write(&message_send, sizeof(message_send));  
  }
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


void moveStepper(bool direction, int gate_ID){
  digitalWrite(DIR, direction);
  if (gate_ID == 1){
    Monitor.println("Moving Stepper #1");
    for(;;){
      digitalWrite(STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(STEP, LOW);
      delayMicroseconds(500);

      if (digitalRead(end_contact1)){
        Monitor.println("End Reached");
   
        radio.stopListening();  //не слушаем радиоэфир, мы передатчик
        radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
      
        const char message_send[32] = "End Reached"; // Adjust the size based on your maximum message size
        radio.write(&message_send, sizeof(message_send));
            
        break;
      }      
    }
    delay(1000);    
  }
  else{
    Monitor.println("Missing Stepper Motor");
  }  
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