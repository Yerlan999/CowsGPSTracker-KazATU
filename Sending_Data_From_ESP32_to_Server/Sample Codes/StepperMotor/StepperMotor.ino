#define Monitor Serial

const int DIR = 32;
const int STEP = 33;

const int CURRENT = 35;

const int  steps_per_rev = 200;

void setup() {
  Monitor.begin(9600); //открываем порт для связи с ПК

  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);

  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

  if (Monitor.available()){
    String input = Monitor.readStringUntil('\n');
    moveStepper(input.toInt());
  }
}


void moveStepper(int rotations){
  digitalWrite(DIR, 1);
  for(int i = 0; i < steps_per_rev*rotations; i++){

    digitalWrite(STEP, HIGH);
    delayMicroseconds(500);
    digitalWrite(STEP, LOW);
    delayMicroseconds(500);  
}
}