#include <ESP8266WiFi.h>  
  
void setup() {  
  Serial.begin(115200);  
  // 等待串口连接  
  while (!Serial) {  
    ; // 等待串口连接  
  }  
  
  // 启动SmartConfig服务  
  Serial.println("Starting SmartConfig");  
  WiFi.beginSmartConfig();  
  
  // 等待SmartConfig完成  
  while (!WiFi.smartConfigDone()) {  
    delay(500);  
    Serial.print(".");  
  }  
  
  // SmartConfig完成后，打印SSID和密码（注意：实际使用中，SSID和密码可能无法直接打印）  
  Serial.println("");  
  Serial.println("SmartConfig done");  
  Serial.print("SSID: ");  
  Serial.println(WiFi.SSID());  
  Serial.print("Password: ");  
  Serial.println(WiFi.psk());  
  
  // 连接到Wi-Fi网络（这一步在SmartConfig完成后通常是自动的，但这里为了演示而显式调用）  
  // 注意：如果你的ESP8266固件版本较新，WiFi.begin()在SmartConfig后可能不需要显式调用  
  // WiFi.begin(WiFi.SSID(), WiFi.psk());  
  
  // 检查Wi-Fi连接状态  
  while (WiFi.status() != WL_CONNECTED) {  
    delay(500);  
    Serial.print(".");  
  }  
  
  Serial.println("");  
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");  
  Serial.println(WiFi.localIP());  
}  
  
void loop() {  
  // 在这个例子中，loop()函数是空的，因为一旦连接上Wi-Fi，就没有更多的操作需要执行  
  // 你可以在这里添加你的代码来处理网络连接后的任务  
  delay(1000);  
}