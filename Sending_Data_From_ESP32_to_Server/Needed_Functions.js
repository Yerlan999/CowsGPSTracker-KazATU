void sendHttpPostRequest() {
  HTTPClient http;

  // Create an array of dictionaries
  DynamicJsonDocument jsonDocument(2048);  // Adjust the size based on your data
  JsonArray jsonArray = jsonDocument.to<JsonArray>();

  // Example dictionaries
  JsonObject dict1 = jsonArray.createNestedObject();
  dict1["latitude"] = 54.214284;
  dict1["longitude"] = 69.514049;
  dict1["index"] = "1";

  JsonObject dict2 = jsonArray.createNestedObject();
  dict2["latitude"] = 54.214440;
  dict2["longitude"] = 69.511097;
  dict2["index"] = "2";

  JsonObject dict3 = jsonArray.createNestedObject();
  dict3["latitude"] = 54.213070;
  dict3["longitude"] = 69.511260;
  dict3["index"] = "3";

  // Add more dictionaries as needed...

  // Serialize the JSON object to a string
  String postData;
  serializeJson(jsonDocument, postData);


  // Make the POST request
  http.begin("http://" + String(host) + ":" + String(port) + "/ajax/");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    Serial.println(httpResponseCode);
  } else {
    Serial.println("Error on sending POST");
  }

  http.end();
}






void moveStepper(bool direction, int gate_ID){
  digitalWrite(DIR, direction);
  if (gate_ID == 1){
    Monitor.println("Moving Stepper #1");
    for(int i = 0; i<(steps_per_rev*stepper_rotation_count); i++){
      digitalWrite(STEP, HIGH);
      delayMicroseconds(500);
      digitalWrite(STEP, LOW);
      delayMicroseconds(500);
    }
    delay(1000);
  }
  else{
    Monitor.println("Missing Stepper Motor");
  }
}




void displayInfo(String send_text, String received_text, String received_SMS_content){
  display.clearDisplay();

  display.setTextSize(1);   display.setTextColor(WHITE);

  display.setCursor(0, 1);  display.println("S| ");
  display.setCursor(20, 1); display.println(send_text);

  display.setCursor(0, 20);  display.println("R| ");
  display.setCursor(20, 20); display.println(received_text);

  display.setCursor(0, 40);  display.println("SMS| ");
  display.setCursor(30, 40); display.println(received_SMS_content);

  display.display();
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



void updateItems(String input){
  String* splitStrings;
  int splitCount = splitString(input, ':', splitStrings);

  if (splitStrings[0].equals(TIME_UPDATE_COMMAND)){
    Monitor.println("Old Time: " + String(cycle_time) + " || New Time: " + splitStrings[1]);
    cycle_time = splitStrings[1].toInt();
  }

  if (splitStrings[0].equals(GATE_CLOSE_COMMAND)){
    Monitor.println(input);
    moveStepper(1, splitStrings[1].toInt());
  }
  if (splitStrings[0].equals(GATE_OPEN_COMMAND)){
    Monitor.println(input);
    moveStepper(0, splitStrings[1].toInt());
  }

  delete[] splitStrings;
}



unsigned long lastTime = 0;
unsigned long cycle_time = 10000;    // КАЖДЫЕ N секунд

if ((millis() - lastTime) > cycle_time) {
  // Code here
  lastTime = millis();
}




enc1.tick(); // обязательная функция отработки. Должна постоянно опрашиваться

if (enc1.isLeft()){ Monitor.println("Left"); }; // если был поворот направо
if (enc1.isRight()){ Monitor.println("Right"); }; // если был поворот налево
if (enc1.isLeftH()){ Monitor.println("Left Hold"); }; // если был поворот направо с удержанием
if (enc1.isRightH()){ Monitor.println("Right Hold"); }; // если был поворот налево с удержанием
if (enc1.isRelease()){ Monitor.println("Release"); }; // отпускание кнопки (+ дебаунс)
if (enc1.isPress()){ Monitor.println("Press"); }; // Нажатие
