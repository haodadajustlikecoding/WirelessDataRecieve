// #include <WiFi.h>          
// #include <WiFiManager.h>  
// #include <HTTPClient.h>
// #include <queue>
// define led according to pin diagram
int led = 8;

void setup() {
  // initialize digital pin led as an output
  pinMode(led, OUTPUT);
  Serial.begin(256000);  // 配置串口
  Serial.setRxBufferSize(2048);
    // pinMode(Output5VPin,OUTPUT);// 配置GPIO输出5V;但是实际输出的是3v3
    // digitalWrite(Output5VPin,HIGH);
  Serial.println("Start!!!!");

}

void loop() {
  digitalWrite(led, HIGH);   // turn the LED off
  delay(500);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED on
  delay(500);               // wait for a second
  Serial.println("Start!!!!");
}

