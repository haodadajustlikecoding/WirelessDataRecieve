#include <ESP8266WiFi.h>          
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
#include <ESP8266HTTPClient.h>       
// 服务器 IP 地址和端口号
// const char* HostIP = "192.168.64.175"; // 服务器 IP 地址 手机热点
// const char* HostIP = "192.168.137.101"; // 服务器 IP 地址  电脑热点
// const char* HostIP = "127.0.0.1"; // 服务器 IP 地址  电脑热点
String  HostIP = "113.219.237.121"; // 服务器 IP 地址  电脑热点
const int HostPort = 44365;                // 服务器端口号
// const char* url = "http://b5e7e0a1-1234-5678-90ab-cdef01234567.mock.pstmn.io"; // 替换为 Mock Server 的 URL
String url = "http://httpbin.org/get";

const int data_size = 272;
WiFiClient client;
uint8_t macAddr[6]; // 建立保存mac地址的数组。用于以下语句
String DeviceID;
void AutoCnnectWiFi(){

    // 自动连接WiFi。以下语句的参数是连接ESP8266时的WiFi名称
    // wifiManager.autoConnect("XenV102V_AutoCnnectAP");
    // 如果您希望该WiFi添加密码，可以使用以下语句：
   
    // if (ESP.getResetReason() == SYSTEM_RESET) {
    //     // 清除ESP8266所存储的WiFi连接信息
    //     WiFiManager wifiManager;
    //     wifiManager.resetSettings();
    //     Serial.println("ESP8266 WiFi Settings Cleared");
    // }
    // 建立WiFiManager对象
    WiFiManager wifiManager;
    wifiManager.autoConnect("XenV102V_AutoCnnectAP", "12345678");
    // 以上语句中的12345678是连接AutoConnectAP的密码
    // WiFi连接成功后将通过串口监视器输出连接成功信息 
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

    AutoCnnectWiFi();  // 自动连接WIFI
      // 设置时间（假设已经同步过NTP时间）
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}
byte variable[272]; //透传雷达数据
byte FrameHead[4] = {0xF1,0xF2,0xF3,0xF4};
byte FrameTail[4] = {0xF5,0xF6,0xF7,0xF8};

int dataindex = 0;


void loop() {
    // 检查是否有可用的串口数据
    Serial.print("MAC地址");
    Serial.println(DeviceID);
    if(Serial.available()>0){
        byte readbyte = Serial.read();
        // Serial.print(readbyte);
        // Serial.print('\r');
        // Serial.print(FrameHead[0]);
        // Serial.print(readbyte==FrameHead[0]);
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
                          Serial.print(dataindex);
                          Serial.print('\t');
                        }
                      }
                      // 连接到服务器
                      // if (!client.connected()) {
                      //     if (client.connect(HostIP, HostPort)) {
                      //         Serial.println("Connected to server");
                      //     } else {
                      //         Serial.println("Connection to server failed");
                      //         return;
                      //     }
                      // }
                      dataindex = 0; 
                      // 准备发送的数据
                      String dataString = "";
                      for (int i = 0; i < data_size; i++) {
                        dataString += String(variable[i], HEX);
                        if (i < data_size - 1) {
                          dataString += " ";
                        }
                    }
                    time_t now = time(nullptr);
                      // 将设备ID和数据格式化为一个字符串
                    // 构建 GET 请求数据
                    HTTPClient http;
                    String message = "device_id=" + DeviceID + "&timestamp=" + String(now) + "&data=" + dataString;
                    url += "?device_id=" + DeviceID;
                    Serial.println(message);

                    http.begin(client, url);
                    Serial.println("Start HTTP Get URL");
                    int httpResponseCode = http.GET(); // 发送 GET 请求

                    if (httpResponseCode > 0) {
                      String response = http.getString(); // 获取响应体
                      Serial.println(httpResponseCode); // 打印 HTTP 响应码
                      Serial.println(response); // 打印响应体
                    } else {
                      Serial.print("Error on sending GET: ");
                      Serial.println(httpResponseCode); // 打印错误码
                    }

                    // client.print(message);
                    //   // 发送数据到服务器 传统服务器TCP方式传输
                    //   // client.write(message.c_str(), message.length());
                    //   // Serial.print("Sent data to server: ");
                    //   Serial.println(message);
          
                    // // 检查是否有来自服务器的数据
                    // if (client.available()) {
                    //     String serverData = client.readStringUntil('\n');
                    //     Serial.print("Received data from server: ");
                    //     Serial.println(serverData);
                    // }
        }
        }
        }
        }
              
    }
    // else{
      // Serial.print("串口没有输入！");
    // }
    
    // 断开连接（可选）
    // client.stop();
}