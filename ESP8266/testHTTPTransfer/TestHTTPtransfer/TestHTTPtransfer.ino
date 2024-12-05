#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
// const char* serverName = "http://b5e7e0a1-1234-5678-90ab-cdef01234567.mock.pstmn.io"; // 替换为 Mock Server 的 URL
const char* serverName = "http://httpbin.org/get";
String DeviceID;

void AutoCnnectWiFi(){
    WiFiManager wifiManager;
    wifiManager.autoConnect("XenV102V_AutoCnnectAP", "12345678");
    Serial.println("");
    Serial.print("ESP8266 Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    if (WiFi.status() == WL_CONNECTED) {
        DeviceID = WiFi.macAddress();
        Serial.print("MAC地址: ");
        Serial.println(DeviceID);
    } else {
        Serial.println("WiFi 未连接");
    }
}

void setup() {
    Serial.begin(256000); // 配置串口
    AutoCnnectWiFi(); // 自动连接WiFi
    configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // 设置时间（同步 NTP 时间）
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { // 检查是否连接到 Wi-Fi
    HTTPClient http;
    WiFiClient client;
    // WiFiClientSecure client; // 使用 HTTPS
    // client.setInsecure(); // 跳过 SSL 证书验证

    Serial.println("开始连接服务器...");
    Serial.print("请求URL: ");
    time_t now = time(nullptr);
    String message =  String(serverName)+"?fromESP8266=1"+ "&timestamp=" + String(now)
    Serial.println( message);
    http.begin(client, message); // 指定目标 URL
    // http.setTimeout(15000); // 设置 15 秒超时

    int httpResponseCode = http.GET(); // 发送 GET 请求

    if (httpResponseCode > 0) {
      String response = http.getString(); // 获取响应体
      Serial.println(httpResponseCode); // 打印 HTTP 响应码
      Serial.println(response); // 打印响应体
    } else {
      Serial.print("Error on sending GET: ");
      Serial.println(httpResponseCode); // 打印错误码
    }

    http.end(); // 关闭连接
  } else {
    Serial.println("WiFi 未连接，等待重连...");
  }
  delay(5000); // 每 5 秒发送一次请求
}
