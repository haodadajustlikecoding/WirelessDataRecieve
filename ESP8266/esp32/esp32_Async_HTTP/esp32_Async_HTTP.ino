#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <queue>
// 适用于S3 和C3
// 服务器 IP 地址和端口号
String url = "http://xenv102v.iclegend.com/api/sys/records/";

const int data_size = 152;
const int frame_save = 10;
const int QUEUE_SIZE = 100; // 队列的最大大小
const int LEDPin = 8;
uint8_t macAddr[6]; // 建立保存mac地址的数组。用于以下语句
String DeviceID;
// 定义队列和队列大小 FreeRTOS

std::queue<String> UsermessageQueue;
std::queue<String> debugmessageQueue;

#define userDataLen  36 // 4+6*4+2*4
#define debugDataLen 108//9*3*4
byte userData[userDataLen]; //透传用户的雷达数据 4+6*4+2*4
byte debugData[debugDataLen]; // 

byte FrameHead[4] = {0xF1,0xF2,0xF3,0xF4};
byte FrameTail[4] = {0xF5,0xF6,0xF7,0xF8};
String FrameHead_str = "f1 f2 f3 f4";
String FrameTail_str = "f5 f6 f7 f8";


int dataindex = 0;
int frameCount = 0;
int state = 0;
void sendDataTask() {
  WiFiClient client;
  HTTPClient http;
  // 如果队列中有消息，则取出并发送
  if (!UsermessageQueue.empty() && !debugmessageQueue.empty()) {
    http.begin(client, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // // 构建批量数据
    // String userbatchData = "";
    // String debugbatchData = "";
    // for (int i = 0; i < frame_save; i++) {
    //   userbatchData += FrameHead_str+" "+UsermessageQueue.front()+" "+FrameTail_str;
    //   UsermessageQueue.pop();
    //   debugbatchData += FrameHead_str+" "+debugmessageQueue.front()+" "+FrameTail_str;
    //   debugmessageQueue.pop();
    //   if (i < frame_save - 1) {
    //     userbatchData += " ";
    //     debugbatchData += " ";
    //   }
    // }
    String TotalData = "";
    for (int i = 0; i < frame_save; i++) {
      TotalData += FrameHead_str+" "+UsermessageQueue.front()+" "+debugmessageQueue.front()+" "+FrameTail_str;
      UsermessageQueue.pop();
      debugmessageQueue.pop();
      if (i < frame_save - 1) {
        TotalData += " ";
      }
    }
    time_t now = time(nullptr);
    String now_timestamp = String(now) + "000";
    // String sendmessage = "mac=" + DeviceID + "&timestamp=" + now_timestamp + "&userdata=" + userbatchData+ "&debugdata="+debugbatchData;
    String sendmessage = "mac=" + DeviceID + "&timestamp=" + now_timestamp + "&data=" + TotalData; // 原来的请求 mac timestamp data

    unsigned long starttime = millis(); 
    int httpResponseCode = http.POST(sendmessage);
    unsigned long endtime = millis();
    Serial.print("Post Time Cost:");
    Serial.println(endtime - starttime);
    // Serial.println(sendmessage);

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

}

void gatherData(){

    while(dataindex<userDataLen){
      if(Serial.available()>0){
        byte readbyte = Serial.read();
        userData[dataindex++] = readbyte;
      }
    }
    dataindex = 0;
    while(dataindex<debugDataLen){
      if(Serial.available()>0){
        byte readbyte = Serial.read();
        debugData[dataindex++] = readbyte;
      }
    }
    dataindex = 0;

    time_t now = time(nullptr);
    String now_timestamp = String(now) + "000"; // 准备发送的数据 AA timestamp 55 

    // user data transfer
    String UserdataString = "";
    UserdataString = UserdataString + "AA " + now_timestamp + " " + "55 ";
    for (int i = 0; i < userDataLen; i++) {
      UserdataString += String(userData[i], HEX);
      if (i < userDataLen - 1) {
        UserdataString += " ";
      }
  }

    // debug data transfer
    String DebugdataString = "";
    DebugdataString = DebugdataString + "AA " + now_timestamp + " " + "55 ";
    for (int i = 0; i < debugDataLen; i++) {
      DebugdataString += String(debugData[i], HEX);
      if (i < debugDataLen - 1) {
        DebugdataString += " ";
      }
  }
    // 将设备ID和数据格式化为一个字符串
  Serial.println(now_timestamp);
  if ((UsermessageQueue.size() < QUEUE_SIZE)&&(debugmessageQueue.size() < QUEUE_SIZE)) {
      UsermessageQueue.push(UserdataString);
      debugmessageQueue.push(DebugdataString);
      frameCount++;
      if (frameCount >= frame_save) {
        frameCount = 0;
        sendDataTask();
      }
    } else {
      Serial.println("Queue full, message dropped!");
  }
      digitalWrite(LEDPin,HIGH);
     

}
void collectDataTask() {
  while (true) {
    if(Serial.available()>0){
        byte readbyte = Serial.read();
        Serial.print(readbyte);
        Serial.print('\r');
        if (state == 0 && readbyte == FrameHead[0]){
          state = 1;
          continue;}
        if (state == 1 && readbyte==FrameHead[1]){
          state = 2;
          continue;
        }
        if (state == 2 && readbyte == FrameHead[2]){
          state = 3;
          continue;
        }
        if (state == 3 && readbyte == FrameHead[3] ){
          state = 0;
          digitalWrite(LEDPin,LOW); // 关闭LED
          gatherData();
        }
    }

  }
}

void AutoCnnectWiFi(){
    WiFiManager wifiManager;
    wifiManager.autoConnect("XenV102V_AutoCnnectAP", "12345678");
    // WiFi连接成功打印信息
    Serial.println(""); 
    Serial.print("ESP32 Connected to ");
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
    digitalWrite(LEDPin,HIGH);
}

void loop() {
  collectDataTask();
}




