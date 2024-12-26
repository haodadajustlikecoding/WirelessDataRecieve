#include <ESP8266WiFi.h>          
#include <WiFiManager.h>  
#include <ESP8266HTTPClient.h>
#include <Ticker.h>
#include <queue>
// 服务器 IP 地址和端口号
// String url = "http://httpbin.org/post"; //post请求 测试
// String url = "http://httpbin.org/get"; //get 请求
String url = "http://xenv102v.iclegend.com/api/sys/records/";

const int data_size = 272;
WiFiClient client;
HTTPClient http;
uint8_t macAddr[6]; // 建立保存mac地址的数组。用于以下语句
String DeviceID;
// 定义队列和队列大小 FreeRTOS
#define QUEUE_SIZE 10
std::queue<String> messageQueue;
// 定义定时器
Ticker httpTicker; //ticker 方式不行

void sendHttpRequests() {
  WiFiClient client;
  HTTPClient http;

  while (true) {
    // 如果队列中有消息，则取出并发送
    if (!messageQueue.empty()) {
      String message = messageQueue.front();
      messageQueue.pop();

      http.begin(client, url);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      int httpResponseCode = http.POST(message);
      if (httpResponseCode > 0) {
        Serial.println(httpResponseCode); // 打印 HTTP 响应码
        // String response = http.getString(); // 获取响应体
        // Serial.println(response); // 打印响应体
      } else {
        Serial.print("Error on sending GET: ");
        Serial.println(httpResponseCode); // 打印错误码
      }
      http.end();
    }
    else{
      break;
    }
    // delay(100);  // 延时，避免占用过多 CPU
  }
}

void AutoCnnectWiFi(){
    WiFiManager wifiManager;
    wifiManager.autoConnect("XenV102V_AutoCnnectAP", "12345678");
    // WiFi连接成功打印信息
    Serial.println(""); 
    Serial.print("ESP8266 Connected to ");
    Serial.println(WiFi.SSID());              // WiFi名称
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());           // IP
    if (WiFi.status() == WL_CONNECTED)
      {
        DeviceID = WiFi.macAddress();   
        Serial.print("MAC地址");
        Serial.println(DeviceID);
    }
  }
void setup() {
    Serial.begin(256000);  // 配置串口
    Serial.setRxBufferSize(2048);
    AutoCnnectWiFi();  // 自动连接WIFI
      // 设置时间（假设已经同步过NTP时间）
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    httpTicker.attach(5, sendHttpRequests);  // 定时 1 秒调用一次 sendHttpRequests

}


byte variable[272]; //透传雷达数据
byte FrameHead[4] = {0xF1,0xF2,0xF3,0xF4};
byte FrameTail[4] = {0xF5,0xF6,0xF7,0xF8};

int dataindex = 0;

void loop() {
    if(Serial.available()>0){
        byte readbyte = Serial.read();
        Serial.print(readbyte);
        Serial.print('\r');
        if (readbyte == FrameHead[0]){
          byte readbyte = Serial.read();
          if (readbyte == FrameHead[1]){
            byte readbyte = Serial.read();
            if (readbyte == FrameHead[2]){
              byte readbyte = Serial.read();
              if (readbyte == FrameHead[3]){
                      variable[dataindex++] =  FrameHead[0];
                      variable[dataindex++] =  FrameHead[1];
                      variable[dataindex++] =  FrameHead[2];
                      variable[dataindex++] =  FrameHead[3];
                      while(dataindex<data_size){
                        if(Serial.available()>0){
                          byte readbyte = Serial.read();
                          variable[dataindex++] = readbyte;
                        }
                      }
                      time_t now = time(nullptr);
                      String now_timestamp = String(now) + "000";

                      // 准备发送的数据 AA timestamp 55 
                      String dataString = "";
                      dataString = dataString + "AA " + now_timestamp + " " + "55 ";
                      for (int i = 0; i < data_size; i++) {
                        dataString += String(variable[i], HEX);
                        if (i < data_size - 1) {
                          dataString += " ";
                        }
                    }
                      // 将设备ID和数据格式化为一个字符串
                    
                    String message = "mac=" + DeviceID + "&timestamp=" + now_timestamp + "&data=" + dataString;
                    Serial.println(now_timestamp);
                    if (messageQueue.size() < QUEUE_SIZE) {
                        messageQueue.push(message);
                      } else {
                        Serial.println("Queue full, message dropped!");
                      }
                    dataindex = 0; 
        }
        }
        }
        }
              
    }
}




