#define M0 12       
#define M1 13

void setup() {
  Serial2.begin(9600);   // lora E32 gắn với cổng TX2 RX2 trên board ESP32
  Serial.begin(9600);
  
  pinMode(M0, OUTPUT);        
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW);       // Set 2 chân M0 và M1 xuống LOW 
  digitalWrite(M1, LOW);       // để hoạt động ở chế độ Normal
  
}

void loop() {
    if(Serial.available() > 0){ // nhận dữ liệu từ bàn phím gửi tín hiệu đi
      String input = Serial.readString();
      Serial2.println(input);     
    }
  
    if(Serial2.available() > 1){
      String input = Serial2.readString();
      Serial.println(input);    
    }
    delay(20);
}
