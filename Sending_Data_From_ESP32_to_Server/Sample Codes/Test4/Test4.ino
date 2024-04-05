#include "Arduino.h"
#include "LoRa_E32.h"


LoRa_E32 LoRa(&Serial2, 15, 22, 21); //  Serial Pins, AUX, M0, M1


void setup() {
  Serial.begin(9600);
  delay(500);

  // Startup all pins and UART
  LoRa.begin();

//  If you have ever change configuration you must restore It
	ResponseStructContainer c;
	c = LoRa.getConfiguration();
	Configuration configuration = *(Configuration*) c.data;
	// Serial.println(c.status.getResponseDescription());
	configuration.CHAN = 0x17;
  configuration.ADDH = 0;
  configuration.ADDL = 1;
	configuration.OPTION.fixedTransmission = 1; // FT_TRANSPARENT_TRANSMISSION = 3
	LoRa.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  c.close();
}

void loop() {
	// If something available
  if (LoRa.available()>1) {
	  // read the String message
	ResponseContainer rc = LoRa.receiveMessageUntil('\n');
	// Is something goes wrong print error
	if (rc.status.code!=1){
		rc.status.getResponseDescription();
	}else{
		// Print the data received
    String input = rc.data;
    input.trim();
    Serial.println(input);    
	}

  }
  if (Serial.available()) {
    String userMessage = Serial.readStringUntil('\n');
    userMessage.trim();

    if (userMessage.startsWith("B:")) { LoRa.sendFixedMessage(0xFF, 0xFF, 0x17, userMessage.substring(2).c_str()); }
    if (userMessage.startsWith("C1:")) { LoRa.sendFixedMessage(0x00, 0x01, 0x17, userMessage.substring(3).c_str()); }
    if (userMessage.startsWith("C2:")) { LoRa.sendFixedMessage(0x00, 0x02, 0x17, userMessage.substring(3).c_str()); }
    
	  // LoRa.sendMessage(input);
    // LoRa.sendBroadcastFixedMessage(0x17, input);
    // LoRa.sendFixedMessage(0xFF, 0xFF, 0x17, input);
    // LoRa.sendFixedMessage(0x00, 0x02, 0x17, input);

  }
}

