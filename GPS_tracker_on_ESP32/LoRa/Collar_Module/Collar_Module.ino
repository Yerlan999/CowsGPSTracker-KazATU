#define M0 19      
#define M1 23
const int RELAY_PIN  = 22;

int relayState = false;

void setup() {
  Serial2.begin(9600);
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);

  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);  
}

void loop() {

  if(Serial.available() > 0){
    String input = Serial.readStringUntil('\n');
    Serial2.println(input);     
  }
 
  if(Serial2.available() > 1){
    String input = Serial2.readStringUntil('\n');
    Serial.println(input);
    
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState);    
          
  }
  delay(20);
}
