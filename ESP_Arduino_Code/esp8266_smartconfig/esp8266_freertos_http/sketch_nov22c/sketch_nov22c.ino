  #include <ESP8266WiFi.h>          
  #include <WiFiManager.h>  
  #include <ESP8266HTTPClient.h>
  #include <queue>
  #include <FreeRTOS.h> // 8266不支持使用
  #include <Arduino_FreeRTOS.h>
  #include <task.h>  
  // 服务器 IP 地址和端口号
  // String url = "http://httpbin.org/post"; //post请求 测试
  // String url = "http://httpbin.org/get"; //get 请求
  String url = "http://xenv102v.iclegend.com/api/sys/records/";

  const int data_size = 272;
  const int frame_count = 10;
  const int QUEUE_SIZE = 100; // 队列的最大大小
  // 发送任务句柄
  TaskHandle_t sendDataTaskHandle = nullptr;
  uint8_t macAddr[6]; // 建立保存mac地址的数组。用于以下语句
  String DeviceID;
  // 定义队列和队列大小 FreeRTOS
  #define QUEUE_SIZE 10
  std::queue<String> messageQueue;
// 数据收集任务
  byte variable[272]; //透传雷达数据
  byte FrameHead[4] = {0xF1,0xF2,0xF3,0xF4};
  byte FrameTail[4] = {0xF5,0xF6,0xF7,0xF8};
  int dataindex = 0;
  int frameCount = 0;
  void collectDataTask(void *parameter) {
  while (true) {
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
                          frameCount++;
                          if (frameCount >= frame_count) {
                            frameCount = 0;
                            xTaskNotifyGive(sendDataTaskHandle);
                          }
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
  }
  void sendDataTask() {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    WiFiClient client;
    HTTPClient http;

    while (true) {
      // 如果队列中有消息，则取出并发送
      if (!messageQueue.empty()) {
        String message = messageQueue.front();
        messageQueue.pop();

        http.begin(client, url);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        // 构建批量数据
        String batchData = "";
        for (int i = 0; i < frame_count; i++) {
          batchData += messageQueue.front();
          messageQueue.pop();
          if (i < frame_count - 1) {
            batchData += "\n";
          }
        }
        time_t now = time(nullptr);
        String now_timestamp = String(now) + "000";

        String sendmessage = "mac=" + DeviceID + "&timestamp=" + now_timestamp + "&data=" + batchData;

        int httpResponseCode = http.POST(sendmessage);
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
      
    // 创建任务
    xTaskCreatePinnedToCore(
      collectDataTask,   /* Task function. */
      "collectDataTask", /* name of task. */
      2048,              /* Stack size of task */
      NULL,              /* parameter of the task */
      1,                 /* priority of the task */
      NULL,              /* Task handle to keep track of created task */
      0);                /* core where the task should run */

    xTaskCreatePinnedToCore(
      sendDataTask,      /* Task function. */
      "sendDataTask",    /* name of task. */
      2048,              /* Stack size of task */
      NULL,              /* parameter of the task */
      1,                 /* priority of the task */
      &sendDataTaskHandle, /* Task handle to notify the task */
      0);                /* core where the task should run */
  }

  


  // byte variable[272]; //透传雷达数据
  // byte FrameHead[4] = {0xF1,0xF2,0xF3,0xF4};
  // byte FrameTail[4] = {0xF5,0xF6,0xF7,0xF8};

  // int dataindex = 0;

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
                          messageQueue.push(dataString);
                        } else {
                          Serial.println("Queue full, dataString dropped!");
                        }
                      dataindex = 0; 
          }
          }
          }
          }
                
      }
  }




