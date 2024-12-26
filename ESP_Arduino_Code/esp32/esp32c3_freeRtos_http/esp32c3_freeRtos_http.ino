//////////////////////
/**    ESP32 C3   **/
// FreeRTOS调试
// #include <FreeRTOS.h> // ESP32 s3支持 但是esp32c3 mini找不到这个，不知道为什么 
// #include <Arduino.h>
#include <WiFi.h>          
#include <WiFiManager.h>  
#include <HTTPClient.h>
#include <queue>

// 服务器 IP 地址和端口号
// 服务器 IP 地址和端口号
String url = "http://xenv102v.iclegend.com/api/sys/records/";

const int data_size = 152;
const int frame_save = 10;
const int QUEUE_SIZE = 1000; // 队列的最大大小
const int LEDPin = 8; // S3 的pin是GPIO2 C3mini的Pin是GPIO8
const int Output5VPin = 18;
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
// 发送任务句柄
TaskHandle_t sendDataTaskHandle = nullptr;
void AutoConnectWiFi(){
    WiFiManager wifiManager;
    wifiManager.autoConnect("XenV102V_AutoCnnectAP", "12345678");
    // WiFi连接成功打印信息
    Serial.println(""); 
    Serial.print("ESP32C3 Connected to ");
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
      // digitalWrite(LEDPin,HIGH);

      if (frameCount >= frame_save) {
        // 确保任务句柄有效
        if (sendDataTaskHandle != NULL) {
            // 发送通知
            xTaskNotifyGive(sendDataTaskHandle);
        } else {
            Serial.println("sendDataTaskHandle is invalid");
        }
        frameCount = 0;
      }
    } else {
      Serial.println("Queue full, message dropped!");
  }     
}

void collectDataTask(void *pvParameters) {
  while (true) {
    // Serial.println("call collectDataTask");
    if (WiFi.status() == WL_CONNECTED)
      {
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
            // unsigned long starttime1 = millis();
            gatherData();
            digitalWrite(LEDPin,LOW);

            // unsigned long starttime2 = millis();
            // Serial.print("Post Time Cost:");
            // Serial.println(starttime2 - starttime1);
          }
      }
      // else{
      //   delay(10); // 可以适当延时，避免高占用CPU
      // }

    }
    else{
      for(int ledIdx = 0;ledIdx<20;ledIdx++)
      {
        digitalWrite(LEDPin,HIGH);  // turn the LED off LED的高电平是关
        delay(500);
        digitalWrite(LEDPin,LOW);
        delay(500);
      }
      digitalWrite(LEDPin,HIGH);

      Serial.println("WIFI Connect Failed！ Reconnecting！");
      AutoConnectWiFi();
      delay(3000);

    }

  }
  vTaskDelete( NULL );

}

void sendDataTask(void *pvParameters) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);

  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //等待通知
  // 如果队列中有消息，则取出并发送
  if (!UsermessageQueue.empty() && !debugmessageQueue.empty()) {
    http.begin(client, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // // 构建批量数据
    String userbatchData = "";
    String debugbatchData = "";
    for (int i = 0; i < frame_save; i++) {
      userbatchData += FrameHead_str+" "+UsermessageQueue.front()+" "+FrameTail_str;
      UsermessageQueue.pop();
      debugbatchData += FrameHead_str+" "+debugmessageQueue.front()+" "+FrameTail_str;
      debugmessageQueue.pop();
      if (i < frame_save - 1) {
        userbatchData += " ";
        debugbatchData += " ";
      }
    }
    // String TotalData = "";
    // for (int i = 0; i < frame_save; i++) {
    //   TotalData += FrameHead_str+" "+UsermessageQueue.front()+" "+debugmessageQueue.front()+" "+FrameTail_str;
    //   UsermessageQueue.pop();
    //   debugmessageQueue.pop();
    //   if (i < frame_save - 1) {
    //     TotalData += " ";
    //   }
    // }
    time_t now = time(nullptr);
    String now_timestamp = String(now) + "000";
    String sendmessage = "mac=" + DeviceID + "&timestamp=" + now_timestamp + "&user_data=" + userbatchData+ "&all_data="+debugbatchData+"&data=0";
    // String sendmessage = "mac=" + DeviceID + "&timestamp=" + now_timestamp + "&data=" + TotalData; // 原来的请求 mac timestamp data

      unsigned long starttime = millis(); 
      http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // HTTP 请求头设置是必须每次发送请求时都设置的，因为每次请求都是独立的，HTTPClient 不会自动记住之前的请求头
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
      http.end(); // 释放资源，因为http不会复用连接，只会创建一个新的
    }
  }
  vTaskDelete( NULL );
}

void setup() {
    delay(1000);
    Serial.begin(256000);  // 配置串口
    pinMode(LEDPin, OUTPUT);
    Serial.setRxBufferSize(4096);
    // pinMode(Output5VPin,OUTPUT);// 配置GPIO输出5V;但是实际输出的是3v3
    // digitalWrite(Output5VPin,HIGH);
    AutoConnectWiFi();  // 自动连接WIFI
      // 设置时间（假设已经同步过NTP时间）
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    
    // 创建任务
    Serial.println(WiFi.status());
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WIFI Connected!");
      Serial.print("Free heap: ");
      Serial.println(ESP.getFreeHeap());

      BaseType_t taskCreateResult2 = xTaskCreate(
          sendDataTask,
          "sendDataTask",
          4096,
          NULL,
          1,
          &sendDataTaskHandle
        );
      Serial.print("Task create result: ");
      Serial.println(taskCreateResult2);  // 打印任务创建的返回值
      if (taskCreateResult2!= pdPASS) {
            Serial.println("Failed to create sendDataTask");
        }   else{
              Serial.println("Success creating sendDataTask");
            }   


      BaseType_t taskCreateResult1 = xTaskCreate(
          collectDataTask,   /* Task function. */
          "collectDataTask", /* Name of task. */
          4096,              /* Stack size of task. */
          NULL,              /* Parameter of the task. */
          1,                 /* Task priority. */
          NULL);                

      Serial.print("Task create result: ");
      Serial.println(taskCreateResult1);  // 打印任务创建的返回值
      if (taskCreateResult1!= pdPASS) {
            Serial.println("Failed to create collectDataTask");
        }else{
        Serial.println("Success creating collectDataTask");
              }

      // BaseType_t taskCreateResult2 = xTaskCreatePinnedToCore(
      //       sendDataTask,      /* Task function. */
      //       "sendDataTask",    /* name of task. */
      //       8192,              /* Stack size of task *///任务堆栈的大小，以字节为单位
      //       NULL,              /* parameter of the task */
      //       1,                 /* priority of the task */
      //       &sendDataTaskHandle, /* Task handle to notify the task */
      //       0) ;

  }    else{
      Serial.println("WIFI NOT Connected!");
    }


  digitalWrite(LEDPin,HIGH);

  Serial.print("Free Heap Memory: ");
  Serial.println(ESP.getFreeHeap());

  Serial.print("Max Allocated Heap Memory: ");
  Serial.println(ESP.getMaxAllocHeap());

}


// int dataindex = 0;

void loop() {
}




