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

String COW_ID = "1";
int cowId = COW_ID.toInt()-1;

float latitude;
float longitude;

char latitudeStr[15];
char longitudeStr[15];


bool ENABLE_DEEP_SLEEP = true;
bool ENABLE_PSM_LORA = true;
bool ENABLE_PSM_GPS = true;

bool DUMMY_MODE = false;
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


// GNSS stoppen
uint8_t gnssStopCmd[] = {   0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x00, 0x00, 0x00, 0x53, 0x54, 0x4F, 0x50, 0xAB, 0x26};
// GNSS running
uint8_t gnssRunningCmd[] = {0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x00, 0x00, 0x00, 0x52, 0x55, 0x4E, 0x20, 0x7A, 0xF3};

// Full power
uint8_t fullPowerCmd[] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x6C};
// Balanced
uint8_t balancedCmd[] = { 0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x95, 0xCD};
// Aggressive 1Hz
uint8_t aggressiveCmd[] ={0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0xDB};

// Software Backup
uint8_t softwareBackupCmd[] = {0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x4B, 0x43, 0x42, 0x86, 0x46};

// Continuous mode
uint8_t continuousModeCmd[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x00, 0x19, 0x68};
// Power save mode
uint8_t powerSaveModeCmd[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x01, 0x1A, 0x69};


void GPS_SendConfig(const uint8_t *Progmem_ptr, size_t arraysize) {
    uint8_t byteread;

    Serial.print(F("GPSSend  "));
    
    for (size_t index = 0; index < arraysize; index++) {
        byteread = pgm_read_byte_near(Progmem_ptr++);
        if (byteread < 0x10) {
            Serial.print(F("0"));
        }
        Serial.print(byteread, HEX);
        Serial.print(F(" "));
    }

    Serial.println();
    Progmem_ptr = Progmem_ptr - arraysize;  // Set Progmem_ptr back to start

    for (size_t index = 0; index < arraysize; index++) {
        byteread = pgm_read_byte_near(Progmem_ptr++);
        GPS.write(byteread);
    }
    delay(100);
}


int num_coordinates = sizeof(dummy_cyclic_coordinates) / sizeof(dummy_cyclic_coordinates[0]);
int counter = 0;

  
void setup() {
  Monitor.begin(9600);

  GPS.begin(9600, SERIAL_8N1, 4, 2); // RX, TX;
  if (ENABLE_PSM_GPS) { 
    GPS_SendConfig(gnssStopCmd, sizeof(gnssStopCmd));
    GPS_SendConfig(aggressiveCmd, sizeof(aggressiveCmd));
    GPS_SendConfig(powerSaveModeCmd, sizeof(powerSaveModeCmd)); 
  };

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
        if (gps.satellites.isValid()) {
            // Get the number of satellites connected
            int numSatellites = gps.satellites.value();
            // Print the number of satellites to the monitor
            Monitor.println("Satellites connected: " + String(numSatellites));
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
  get_GPS_coordinates();
  LoRa.setMode(MODE_0_NORMAL);
  ResponseStatus responce = writeToLoRa(String(COW_ID) + " | " + String(latitudeStr) + " | " + String(longitudeStr));
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