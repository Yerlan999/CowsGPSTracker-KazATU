#include <Preferences.h>

String ssid;
String password;

Preferences preferences;

void setup() {
  Serial.begin(9600);

  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");

  bool gate1_state = preferences.getBool("gate1");
  bool gate2_state = preferences.getBool("gate2");
  bool gate3_state = preferences.getBool("gate3");
  bool gate4_state = preferences.getBool("gate4");
  bool gate5_state = preferences.getBool("gate5");
  bool gate6_state = preferences.getBool("gate6");
  bool gate7_state = preferences.getBool("gate7");

  int current_timing = preferences.getInt("time");

  String massive_message = String(current_timing) + "|" + String(gate1_state) + "|" + String(gate2_state) + "|" + String(gate3_state) + "|" + String(gate4_state) + "|" + String(gate5_state) + "|" + String(gate6_state) + "|" + String(gate7_state);

  Serial.println(massive_message);
  Serial.println(ssid);
  Serial.println(password);



}

void loop() {
  // put your main code here, to run repeatedly:

}
