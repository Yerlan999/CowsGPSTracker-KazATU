#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <string.h>
#include <Arduino.h>
#include <Wire.h>

#define M0 18
#define M1 19

HardwareSerial LoRa(1);  // use UART1
TinyGPSPlus gps;

#define GPS Serial2  // Ublox NEO-M8N
// GREEN - TX (GPS) --> RX2 (ESP32);
// YELLOW - RX (GPS) --> TX2 (ESP32)
#define Monitor Serial

String COW_ID = "1";

float latitude;
float longitude;

unsigned long lastTime = 0;
unsigned long cycle_time = 10;  // КАЖДЫЕ N секунд

String GPS_ENABLE_COMMAND = "START GPS";
String GPS_DISABLE_COMMAND = "STOP GPS";
String TIME_UPDATE_COMMAND = "TIME_UPDATE";

bool deliver_GPS = false;


float dummy_coordinates[][2] = {
  { 54.214802, 69.511022 },
  { 54.214422, 69.510974 },
  { 54.214013, 69.511046 },
  { 54.213562, 69.511287 },
  { 54.213224, 69.511335 },
  { 54.212604, 69.511408 },
  { 54.212067, 69.511659 },
  { 54.211783, 69.512166 },
  { 54.211891, 69.512238 },
  { 54.212285, 69.512249 },
  { 54.212515, 69.512424 },
  { 54.212793, 69.512559 },
  { 54.213234, 69.513128 }
};

int numCoordinates = sizeof(dummy_coordinates) / sizeof(dummy_coordinates[0]);  // Calculate the number of coordinates
int currentCoordinateIndex = 0;  // Initialize the index counter


void setup() {

  LoRa.begin(9600, SERIAL_8N1, 4, 2);
  GPS.begin(9600);  // lora E32 gắn với cổng TX2 RX2 trên board ESP32
  Monitor.begin(9600);

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);  // Set 2 chân M0 và M1 xuống LOW
  digitalWrite(M1, LOW);  // để hoạt động ở chế độ Normal
}

void loop() {

  if (Monitor.available() > 0) {  // nhận dữ liệu từ bàn phím gửi tín hiệu đi
    String input = Monitor.readStringUntil('\n');
    LoRa.println(input);
  }

  if (LoRa.available() > 0) {
    String input = LoRa.readStringUntil('\n');
    input.trim();

    Monitor.println(input);

    if (input.equals(GPS_ENABLE_COMMAND)) {
      deliver_GPS = true;
    }
    if (input.equals(GPS_DISABLE_COMMAND)) {
      deliver_GPS = false;
    }

    updateTime(input);
  }


  if ((millis() - lastTime) > cycle_time * 1000) {
    if (deliver_GPS) {
      get_GPS_coordinates();

      // GETTING DUMMY COORDINATES FOR A WHILE
      latitude = dummy_coordinates[currentCoordinateIndex][0];
      longitude = dummy_coordinates[currentCoordinateIndex][1];
      currentCoordinateIndex = (currentCoordinateIndex + 1) % numCoordinates;
      
      char latitudeStr[15];  // Adjust the size based on your precision needs
      char longitudeStr[15];

      // Convert latitude and longitude to strings with 6 decimal places
      dtostrf(latitude, 6, 6, latitudeStr);
      dtostrf(longitude, 6, 6, longitudeStr);

      LoRa.println(String(COW_ID) + " | " + String(latitudeStr) + " | " + String(longitudeStr));

      lastTime = millis();
    }
  }

  delay(20);
}


void updateTime(String input) {
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);

  if (splitStrings[0].equals(TIME_UPDATE_COMMAND)) {
    Monitor.println("Old Time: " + String(cycle_time) + " || New Time: " + splitStrings[1]);
    cycle_time = splitStrings[1].toInt();
  }
  delete[] splitStrings;
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

void get_GPS_coordinates() {
  while (GPS.available() > 0) {
    if (gps.encode(GPS.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        // Monitor.println(String(latitude) + " | " + String(longitude));
      } else {
        // Monitor.println("Invalid Location");
        // Monitor.println(String(latitude) + " | " + String(longitude));
      }
    }
  }
}