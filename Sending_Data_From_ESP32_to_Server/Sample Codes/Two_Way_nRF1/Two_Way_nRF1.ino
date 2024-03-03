#include "SPI.h" 
#include "RF24.h" 
#include "nRF24L01.h" 

#define CE_PIN 4 
#define CSN_PIN 5 

RF24 nRF24(CE_PIN, CSN_PIN); 

const byte address[2][6] = {"00001", "00002"};
const int listen_to = 1;
const int write_into = 0;

void setup() 
{ 
	 Serial.begin(9600); 
	 nRF24.begin(); 
	 //Append ACK packet from the receiving nRF24 back to the transmitting nRF24 
	 nRF24.setAutoAck(true); //(true|false) 
	 //Set the transmission datarate 
	 nRF24.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS) 
	 //Greater level = more consumption = longer distance 
	 nRF24.setPALevel(RF24_PA_MIN); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX) 
	 //Default value is the maximum 32 bytes1 
	 nRF24.setRetries(0, 15);
   nRF24.enableDynamicPayloads();
   nRF24.enableAckPayload();
   nRF24.setPayloadSize(32);
	 //Act as receiver 
	 nRF24.openReadingPipe(1, address[listen_to]);
   nRF24.openWritingPipe(address[write_into]); 
   
   nRF24.startListening();
} 


void loop() {
  
  listenToNRF24();

  if (Serial.available()){
    String input = Serial.readStringUntil('\n');
    writeIntoNRF24(input);
  }

}

void writeIntoNRF24(String inputString){
  nRF24.stopListening();
  char input[32];
  inputString.trim();  // Remove leading and trailing whitespaces, if needed
  inputString.toCharArray(input, sizeof(input));

  bool report = nRF24.write(input, sizeof(input));
  nRF24.startListening();   
}
void listenToNRF24(){
  if (nRF24.available()) { 
    readFromNRF24();
  }  
}

String readFromNRF24(){
  char input[32];
  nRF24.read(&input, sizeof(input)); 
  Serial.print("Received: "); 
  Serial.println(input);
  return String(input);   
}