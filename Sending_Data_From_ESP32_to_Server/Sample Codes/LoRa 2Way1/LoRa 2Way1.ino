// HEAD MODULE

#include "Arduino.h"
#include "LoRa_E32.h"
 
LoRa_E32 LoRa(&Serial2, 15, 22, 21); //  RX AUX M0 M1
 
void setup() {
  Serial.begin(9600);
  LoRa.begin(); 
}
 
void loop() {
  
  listenToLoRa();

  if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      ResponseStatus responce = writeToLoRa(input);
      // Serial.println(responce.getResponseDescription());   
  }
}

void listenToLoRa(){
  if (LoRa.available()>1) {
    String input = readFromLoRa();
    Serial.println(input);
  }  
}

String readFromLoRa(){
  ResponseContainer rc = LoRa.receiveMessage();
  if (rc.status.code!=1){
      rc.status.getResponseDescription();
  }else{  
      return rc.data;
  }  
}

ResponseStatus writeToLoRa(String input){
  ResponseStatus rs = LoRa.sendMessage(input);
  return rs;
}