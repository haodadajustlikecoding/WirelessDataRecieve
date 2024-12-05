#include <ESP8266WiFi.h>
// smart config 太难用了
void wifiInit() {
  delay(10);
  Serial.println("\r\nEsp8266 Connecting to WiFi");
  WiFi.begin();
  for (int i = 0; i < 5; i++) {  //5s use autoConfig Connecting to WiFi
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
    } else {
      Serial.println("\r\nAutoConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PWD:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    smartConfig();
    delay(3000);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    for (int i = 0; i < 5; i++) {  //WiFi connected,led flashing 5 times
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }
  } else {
    Serial.println("\r\nWiFi connection failed");
    Serial.println("Please restart and try again");
    digitalWrite(LED_BUILTIN, LOW);  //WiFi connection failed,led on
  }
}
void smartConfig() {
  Serial.println("\nWiFi connection failed");
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  Serial.println("\nWaiting for SmartConfig");
  while (1) {
    Serial.print(".");
    if (WiFi.smartConfigDone()) {
      Serial.println("");
      Serial.println("SmartConfig received");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PWD:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true);
      break;
    }
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  wifiInit();
}

void loop() {
}