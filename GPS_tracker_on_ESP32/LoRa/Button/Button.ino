#define BUTTON_PIN 23
#define M0 21       
#define M1 18

int lastState = HIGH;
int currentState;     

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);

  Serial2.begin(9600);
  
  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);     
  digitalWrite(M1, LOW);  
}

void loop() {
  delay(230);
  currentState = digitalRead(BUTTON_PIN);
  if(lastState == LOW && currentState == HIGH){
    Serial.println("The state changed from LOW to HIGH");
    Serial2.println("SOS");
  }   
  lastState = currentState;
}
