#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <string.h>
#include <Arduino.h>
#include <Wire.h>
#include "LoRa_E32.h"

LoRa_E32 LoRa(&Serial2, 15, 22, 21, UART_BPS_RATE_9600); //  Serial Pins, AUX, M0, M1

HardwareSerial GPS(1);  // use UART1
// GREEN wire - TX (GPS) --> RX (ESP32) (pin 4)
// YELLOW wire - RX (GPS) --> TX (ESP32) (pin 2)
TinyGPSPlus gps;

#define Monitor Serial
#define LED 18

String COW_ID = "2";
int cowId = COW_ID.toInt()-1;

float latitude;
float longitude;

char latitudeStr[15];
char longitudeStr[15];


bool ENABLE_DEEP_SLEEP = false;
bool ENABLE_PSM_LORA = false;
bool ENABLE_PSM_GPS = false;

bool DUMMY_MODE = true;
bool CYCLIC = false;

unsigned long lastTime = 0;
unsigned long cycle_time = 10;  // КАЖДЫЕ N секунд

String GPS_ENABLE_COMMAND = "START GPS" + COW_ID;

bool deliver_GPS = false;


float dummy_coordinates[][2] = {
  {54.217407, 69.509214},
  {54.215474, 69.511096},
  {54.214190, 69.513823},
  {54.213131, 69.512447},
  {54.215313, 69.526096},
};


// Cyclic Cattle Movement Simulation
float dummy_cyclic_coordinates[][2] = {
    {54.214176, 69.511151},
    {54.213195, 69.511303},
    {54.212246, 69.511400},
    {54.211856, 69.512168},
    {54.212011, 69.512357},
    {54.212327, 69.512357},
    {54.212580, 69.512519},
};


// Define the payload for Balanced Power saving mode
const uint8_t balancedPowerSavingPayload[] = {
  0x00, // Version
  0x01, // Reserve1
  0x10, // Reserve2
  0x10, // Reserve3
  0x05, // Power Setup: Balanced Power Saving mode
  0x05, // Period: Not used in Balanced mode (set to 0)
  0x00, // OnTime: Not used in Balanced mode (set to 0)
  0x00, // MinTime: Not used in Balanced mode (set to 0)
};


int num_coordinates = sizeof(dummy_cyclic_coordinates) / sizeof(dummy_cyclic_coordinates[0]);
int counter = 0;

  
void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Monitor.begin(9600);

  GPS.begin(9600, SERIAL_8N1, 4, 2); // RX, TX;
  if (ENABLE_PSM_GPS) { configurePowerManagement(); };
    
  LoRa.begin();
  if (ENABLE_PSM_LORA) { LoRa.setMode(MODE_2_POWER_SAVING); };
  

  ResponseStructContainer c;
  c = LoRa.getConfiguration();
  Configuration configuration = *(Configuration*) c.data;
  configuration.ADDL = 0;
  configuration.ADDH = COW_ID.toInt();
  configuration.CHAN = 23;
  // FT_TRANSPARENT_TRANSMISSION = 1 vs FT_FIXED_TRANSMISSION = 0
  configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
  if (ENABLE_PSM_LORA) { configuration.OPTION.wirelessWakeupTime = WAKE_UP_250; };
  LoRa.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
  // printParameters(configuration);
  c.close();


  if (ENABLE_DEEP_SLEEP){ enableDeepSleep(); }
  
}

void loop() {

  listenToLoRa();

  if (Monitor.available() > 0) {
    String input = Monitor.readStringUntil('\n');
    sendCoordinatesToHead();
  }

  delay(20);
}



void listenToLoRa(){
  if (LoRa.available()>1) {
    String input = readFromLoRa();
    
    input.trim();
    Monitor.println(input);

    if (input.equals(GPS_ENABLE_COMMAND)) {
      
      sendCoordinatesToHead();

    }

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
  ResponseStatus rs = LoRa.sendFixedMessage(1, 1, 23, input);
  return rs;
}



void get_GPS_coordinates() {
  if (DUMMY_MODE) { latitude = dummy_coordinates[cowId][0]; longitude = dummy_coordinates[cowId][1]; 
    if (CYCLIC){ latitude = dummy_cyclic_coordinates[counter][0]; longitude = dummy_cyclic_coordinates[counter][1]; counter = (counter + 1) % num_coordinates; }; 
  }
  else {
    while (GPS.available() > 0) {
      if (gps.encode(GPS.read())) {
        if (gps.location.isValid()) {
          latitude = gps.location.lat(); longitude = gps.location.lng();
          Monitor.println(String(latitude) + " | " + String(longitude));
        } else {
          // Monitor.println("Invalid Location");
          // Monitor.println(String(latitude) + " | " + String(longitude));
        }
      }
    }
  }

  // Convert latitude and longitude to strings with 6 decimal places
  dtostrf(latitude, 6, 6, latitudeStr); dtostrf(longitude, 6, 6, longitudeStr);

}


void printParameters(struct Configuration configuration) {
    Serial.println("----------------------------------------");
 
    Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN);Serial.print(" ");Serial.print(configuration.HEAD, DEC);Serial.print(" ");Serial.println(configuration.HEAD, HEX);
    Serial.println(F(" "));
    Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, DEC);
    Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, DEC);
    Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
    Serial.println(F(" "));
    Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
    Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
    Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());
 
    Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
    Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
    Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
    Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
    Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());
 
    Serial.println("----------------------------------------");
 
}


void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}


void sendCoordinatesToHead(){
  digitalWrite(LED,HIGH);
  get_GPS_coordinates();
  LoRa.setMode(MODE_0_NORMAL);
  ResponseStatus responce = writeToLoRa(String(COW_ID) + " | " + String(latitudeStr) + " | " + String(longitudeStr));
  digitalWrite(LED,LOW);
  LoRa.setMode(MODE_2_POWER_SAVING);  
}


void enableDeepSleepMode(){
  LoRa.setMode(MODE_2_POWER_SAVING);

  delay(1000);
  Monitor.println();
  Monitor.println("Entering in Deep Sleep Mode");
  delay(100);

  if (ESP_OK == gpio_hold_en(GPIO_NUM_22)){ Monitor.println("HOLD 22"); } else { Monitor.println("NO HOLD 22"); }
  if (ESP_OK == gpio_hold_en(GPIO_NUM_21)){ Monitor.println("HOLD 21"); } else{ Monitor.println("NO HOLD 21"); }

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,LOW);

  gpio_deep_sleep_hold_en();
  //Go to sleep now
  Monitor.println("Deep Sleep has been Started!");
  esp_deep_sleep_start();  
}


void configurePowerManagement() {
  // Send the UBX-CFG-PMS message to the GPS module for Balanced Power saving mode
  sendUBXMessage(0x06, 0x86, balancedPowerSavingPayload, sizeof(balancedPowerSavingPayload));
}

void sendUBXMessage(uint8_t msgClass, uint8_t msgId, const uint8_t* payload, uint16_t length) {
  // Send UBX header
  GPS.write(0xB5);
  GPS.write(0x62);

  // Send message class and ID
  GPS.write(msgClass);
  GPS.write(msgId);

  // Send payload length
  GPS.write(length & 0xFF);
  GPS.write(length >> 8);

  // Send payload
  for (int i = 0; i < length; i++) {
    GPS.write(payload[i]);
  }

  // Calculate and send checksum
  uint8_t checksumA = 0, checksumB = 0;
  for (int i = 0; i < length; i++) {
    checksumA += payload[i];
    checksumB += checksumA;
  }
  GPS.write(checksumA);
  GPS.write(checksumB);
}


void enableDeepSleep(){

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  if (ESP_SLEEP_WAKEUP_EXT0 == wakeup_reason) {
    Monitor.println("Waked up from Radio Signal");

    gpio_hold_dis(GPIO_NUM_22);
    gpio_hold_dis(GPIO_NUM_21);
    gpio_deep_sleep_hold_dis();

    sendCoordinatesToHead();
    enableDeepSleepMode();
  }
  else{
    enableDeepSleepMode();
    delay(1);
  }  
}