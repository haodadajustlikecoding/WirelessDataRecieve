# boot.py - - runs on boot-up
import network
import time
from machine import Pin

LED = Pin(2, Pin.OUT)  # 定义LED引脚
def connect_wifi_certain():
    # 连接指定wifi
    ssid = "0301-SH1-0002 6248"
    password = "123456qwe"
    wlan = network.WLAN(network.STA_IF)  # STA模式
    wlan.active(False)  # 先进行wlan的清除
    wlan.active(True)  # 再激活
    start_time = time.time()  # 记录时间做超时判断
    if not wlan.isconnected():
        print("connecting to network…")
        wlan.connect(ssid, password)  # 输入WiFi账号和密码
        while not wlan.isconnected():
            LED.value(1)
            time.sleep_ms(300)
            LED.value(0)
            time.sleep_ms(300)
            # 超时判断，30s未连接成功判定为超时
            if time.time() - start_time > 30:
                print("WiFi Connect TimeOut!")
                break
    if wlan.isconnected():
        LED.value(1)
        print("network information:", wlan.ifconfig())

connect_wifi_certain()  # 连接指定wifi