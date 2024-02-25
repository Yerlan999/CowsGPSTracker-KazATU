#include <Preferences.h>

Preferences preferences;

void setup() {
  preferences.begin("credentials", false);

  preferences.putString("ssid", "Le petit dejeuner 2");
  preferences.putString("password", "DoesGodReallyExist404");

  preferences.putBool("gate1", false);
  preferences.putBool("gate2", false);
  preferences.putBool("gate3", false);
  preferences.putBool("gate4", false);
  preferences.putBool("gate5", false);
  preferences.putBool("gate6", false);
  preferences.putBool("gate7", false);

  preferences.putInt("time", 10);

}

void loop() {
  // put your main code here, to run repeatedly:

}
