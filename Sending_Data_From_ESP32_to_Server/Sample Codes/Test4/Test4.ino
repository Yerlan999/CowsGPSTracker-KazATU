// #include "Arduino.h"
 
// void setup() {
//   Serial.begin(9600);
//   delay(500);
 
//   Serial.println("Hi, I'm going to send message!");
 
//   Serial2.begin(9600);
//   Serial2.println("Hello, world?");
// }
 
// void loop() {
//   if (Serial2.available()) {
//     Serial.write(Serial2.read());
//   }
//   if (Serial.available()) {
//     Serial2.write(Serial.read());
//   }
// }





#include "Arduino.h"
#include "LoRa_E32.h"
 
 
LoRa_E32 LoRa(&Serial2, 15, 22, 21, UART_BPS_RATE_9600); //  Serial Pins, AUX, M0, M1
 
void setup() {

  Serial.begin(9600);
  delay(500);
 
  // Startup all pins and UART
  LoRa.begin();
}
 
void loop() {
    // If something available
  if (LoRa.available()>1) {
      // read the String message
    ResponseContainer rc = LoRa.receiveMessage();
 
    // Is something goes wrong print error
    if (rc.status.code!=1){
        Serial.println(rc.status.getResponseDescription());
    }else{
        // Print the data received
        Serial.println(rc.data);
    }
  }
  if (Serial.available()) {
      String input = Serial.readString();
      LoRa.sendMessage(input);
  }
}










// #include "Arduino.h"
// #include "LoRa_E32.h"


// LoRa_E32 LoRa(&Serial2, 15, 22, 21, UART_BPS_RATE_9600); //  Serial Pins, AUX, M0, M1


// void setup() {
//   Serial.begin(9600);
//   delay(500);

//   // Startup all pins and UART
//   LoRa.begin();

// //  If you have ever change configuration you must restore It
// 	ResponseStructContainer c;
// 	c = LoRa.getConfiguration();
// 	Configuration configuration = *(Configuration*) c.data;
// 	// Serial.println(c.status.getResponseDescription());
// 	configuration.CHAN = 0x17;
//   configuration.ADDH = 0x00;
//   configuration.ADDL = 0x01; 
// 	configuration.OPTION.fixedTransmission = 0; // FT_TRANSPARENT_TRANSMISSION = 3
// 	LoRa.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
//   c.close();
// }

// void loop() {
// 	// If something available
//   if (LoRa.available()>1) {
// 	  // read the String message
// 	ResponseContainer rc = LoRa.receiveMessageUntil('\n');
// 	// Is something goes wrong print error
// 	if (rc.status.code!=1){
// 		rc.status.getResponseDescription();
// 	}else{
// 		// Print the data received
//     String input = rc.data;
//     input.trim();
//     Serial.println(input);  
// 	}

//   }
//   if (Serial.available()) {

//     String userMessage = Serial.readStringUntil('\n');
//     userMessage.trim();

//     if (userMessage.startsWith("B:")) { LoRa.sendFixedMessage(0xFF, 0xFF, 0x17, userMessage.substring(2).c_str()); }
//     if (userMessage.startsWith("C1:")) { LoRa.sendFixedMessage(0x00, 0x01, 0x17, userMessage.substring(3).c_str()); }
//     if (userMessage.startsWith("C2:")) { LoRa.sendFixedMessage(0x00, 0x02, 0x17, userMessage.substring(3).c_str()); }
      
// 	  // LoRa.sendMessage(userMessage);
//     // LoRa.sendBroadcastFixedMessage(0x17, userMessage);
//     // LoRa.sendFixedMessage(0xFF, 0xFF, 0x17, userMessage);
//     // LoRa.sendFixedMessage(0x00, 0x01, 0x17, userMessage);

//   }
// }

