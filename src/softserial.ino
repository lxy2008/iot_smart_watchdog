#include <SoftwareSerial.h>

SoftwareSerial esp_wifi(8, 9); 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  esp_wifi.begin(9600);


  Serial.print("ready!");
  Serial.println();

  
}


void loop() {
  // put your main code here, to run repeatedly:
  int wr = 0,rd = 0;
  char sbuf[64] = {0x0};
  char rbuf[64] = {0x0};

  
  while(Serial.available() > 0){
    sbuf[wr] = Serial.read();
    wr++;
    delay(2);
  }
  if(wr > 0){
    Serial.print(">>>>>>>>>>>>>>>>>>>>>>");
    Serial.println();
    Serial.print(sbuf);
    Serial.println();
    Serial.print("<<<<<<<<<<<<<<<<<<<<<<");
    Serial.println();


    esp_wifi.print(sbuf);
    esp_wifi.println();
  }
  
  //delay(10);
  while (esp_wifi.available()>0){
    rbuf[rd] = esp_wifi.read();
    rd ++;
    delay(2);
  }
  if(rd > 0){
    Serial.print(rbuf);
    Serial.println();
  }
 
}
