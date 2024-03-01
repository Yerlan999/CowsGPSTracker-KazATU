#include "SPI.h" 
#include "RF24.h" 
#include "nRF24L01.h" 

#define CE_PIN 4 
#define CSN_PIN 5 

RF24 radio(CE_PIN, CSN_PIN); 

const byte address[2][6] = {"00001", "00002"};
const int listen_to = 0;
const int write_into = 1;

void setup() 
{ 
	 Serial.begin(9600); 
	 radio.begin(); 
	 //Append ACK packet from the receiving radio back to the transmitting radio 
	 radio.setAutoAck(false); //(true|false) 
	 //Set the transmission datarate 
	 radio.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS) 
	 //Greater level = more consumption = longer distance 
	 radio.setPALevel(RF24_PA_MIN); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX) 
	 //Default value is the maximum 32 bytes1 
	 radio.setPayloadSize(32); 
	 //Act as receiver 
	 radio.openReadingPipe(0, address[listen_to]);
   radio.openWritingPipe(address[write_into]); 
   
   radio.startListening();
} 


void loop() {

   if (Serial.available()){
      radio.stopListening(); 
      String input = Serial.readStringUntil('\n');       
      radio.write(&input, sizeof(input)); 
      radio.startListening(); 
   }
	 if (radio.available()) { 
     String input;
	   radio.read(&input, sizeof(input)); 
	   Serial.print("Received: "); 
	   Serial.println(input); 
	 }
}

