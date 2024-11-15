# main.py
import uasyncio

import machine
from machine import Pin
import time

import network
import socket
import ujson
import time
import network
import time
from machine import Pin
import os
import json
import wifimgr

#  ampy --port COM9 .\src\main.py
LED=Pin(2,Pin.OUT)
SERVER_IP = '192.168.102.40'
SERVER_PORT = 8080
def LED_breatheblink():
    for i in range(0,100):
        LED.off()
        time.sleep(0.0001*i)
        LED.on()
        time.sleep(0.0001*(100-i))
    for i in range(0,100):
        LED.off()
        time.sleep(0.0001*(100-i))
        LED.on()
        time.sleep(0.0001*i)
    # print("blinking")
def LED_shining(time_inverval):
    time.sleep(time_inverval)
    LED.on()
    time.sleep(time_inverval)
    LED.off()

# 创建一个AP热点
def start_ap():
    LED.on() # 创建AP模式时，点亮LED
    ap = network.WLAN(network.AP_IF)
    ap.active(True)
    ap.config(essid="XenV102V_VitalSigns", password="12345678")  # 设置热点名称和密码
    # 等待热点启用
    while ap.active() == False:
        pass
    print("AP mode started, SSID: XenV102V_VitalSigns, IP:", ap.ifconfig())

    LED.off()  # 配网成功后，熄灭LED
    return ap

# 启动Web服务器，等待配网信息
def start_web_server():

    addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1] # 监听所有IP地址的80端口
    s = socket.socket()# 创建套接字
    s.bind(addr)
    s.listen(1) # 开始监听
    print('Web server listening on', addr)
    connect_flag = False  # 标记是否连接到Wi-Fi
    while True:
        cl, addr = s.accept()
        print('Client connected from', addr)
        # cl_file = cl.makefile('rwb', 0)
        while True:
            # LED_shining(0.5)  # 等待配网信息 闪烁模式等待配网信息

            # line = cl_file.readline()
            # if not line or line == b'\r\n':
            #     break
            request = cl.recv(1024)
            print("Request:", request)
                # 读取客户端发送的数据
            # try:
            #     request = cl.recv(1024)
            #     print("Request:", request)
            #
            #     # 如果请求为空，发送错误响应并关闭连接
            #     if not request:
            #         response = "HTTP/1.1 400 Bad Request\r\n\r\nEmpty request."
            #         cl.send(response)
            #         cl.close()
            #         continue
            #
            #     # 解析 JSON 数据
            #     try:
            #         request_data = ujson.loads(request)
            #         ssid = request_data.get('ssid')
            #         password = request_data.get('password')
            #
            #         if ssid and password:
            #             # 返回成功信息
            #             response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nWi-Fi configuration received."
            #             cl.send(response)
            #             cl.close()
            #
            #             # 尝试连接到 Wi-Fi
            #             connect_flag = connect_to_wifi(ssid, password)
            #             break
            #         else:
            #             response = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid data format."
            #             cl.send(response)
            #             cl.close()
            #
            #     except ValueError:
            #         # 解析JSON时发生错误
            #         print("Error: Invalid JSON format")
            #         response = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid JSON format."
            #         cl.send(response)
            #         cl.close()
            #
            # except Exception as e:
            #     print("Error:", e)
            #     response = "HTTP/1.1 500 Internal Server Error\r\n\r\n" + str(e)
            #     cl.send(response)
            #     cl.close()
    return connect_flag

# 连接到指定Wi-Fi
def connect_to_wifi(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(ssid, password)

    # 等待连接
    for _ in range(20):
        if wlan.isconnected():
            print('Connected to Wi-Fi, IP:', wlan.ifconfig())
            LED.off()  # 连接Wi-Fi成功后，熄灭LED
            return True
        time.sleep(1)
    else:
        print('Failed to connect to Wi-Fi.')
        LED_shining(2) # 连接Wi-Fi失败后，点亮LED 2秒
        return False

def tcp_client():
    try:
        # 创建socket对象
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # 连接到服务器
        s.connect((SERVER_IP, SERVER_PORT))
        print("Connected to server")

        while True:
            # 发送数据
            message = "Hello from ESP8266"
            s.send(message.encode('utf-8'))
            print("Sent:", message)

            # 接收数据
            data = s.recv(1024)
            if data:
                print("Received:", data.decode('utf-8'))

            time.sleep(5)  # 每5秒发送一次数据

    except Exception as e:
        print("Error:", e)
        s.close()
        LED.value(0)  # 关闭LED

    finally:
        s.close()
# 主程序入口
def get_data_from_rx_2_tcp():
    """有两个 UART 可用。UART0 位于引脚 1 (TX) 和 3 (RX) 上。UART0 是双向的，默认情况下用于 REPL。
    UART1 位于引脚 2 (TX) 和 8 (RX) 上，但引脚 8 用于连接闪存芯片，因此 UART1 仅用作 TX。"""

    # 初始化UART
    uart = machine.UART(1, baudrate=115200, tx=1, rx=3)  # 根据你的硬件配置调整引脚

    async def send_data(s, queue):
        while True:
            message = await queue.get()
            if message:
                try:
                    s.send(message.encode('utf-8'))
                    print("Sent:", message)
                except Exception as e:
                    print("Error sending data:", e)
            await uasyncio.sleep(0.1)

    async def receive_uart(uart, queue):
        while True:
            if uart.any():  # 检查是否有数据
                data = uart.read()  # 读取所有可用的数据
                if data:
                    message = data.decode('utf-8').strip()
                    print("UART Received:", message)
                    await queue.put(message)  # 异步放入队列
            await uasyncio.sleep(0.1)

    async def main():
        try:
            # 创建 socket 并连接到服务器
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((SERVER_IP, SERVER_PORT))
            print("Connected to server")

            # 创建队列
            queue = uasyncio.Queue()

            # 创建异步任务
            tasks = [
                uasyncio.create_task(send_data(s, queue)),
                uasyncio.create_task(receive_uart(uart, queue))
            ]

            # 运行所有异步任务
            await uasyncio.gather(*tasks)

        except Exception as e:
            print("Error:", e)
            if s:
                s.close()
        finally:
            if s:
                s.close()

    # 启动主程序
    uasyncio.run(main())

def get_data_from_uart2tcp(uart):
    async def receive_uart_and_send_tcp(sock):
        """从 UART 接收数据并通过 TCP 发送出去"""
        while True:
            if uart.any():  # 检查是否有 UART 数据可读
                data = uart.read()  # 从 UART 读取数据
                if data:
                    try:
                        print("UART Received:", data)
                        # 通过 TCP 发送数据到服务器
                        sock.send(data)
                        print("Data sent to server:", data)
                    except Exception as e:
                        print("TCP Send Error:", e)
                        break
            await uasyncio.sleep(0.1)  # 控制循环频率，避免占用过多资源
            sock.send("Waiting for data...")
            print("Waiting for data...")


    async def main():
        try:
            # 创建 TCP 客户端 socket 并连接到服务器
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((SERVER_IP, SERVER_PORT))
            print("Connected to server")

            # 启动 UART 数据接收并通过 TCP 透传
            await receive_uart_and_send_tcp(sock)

        except Exception as e:
            print("Connection Error:", e)

        finally:
            # 关闭 TCP 连接
            sock.close()
            print("Connection closed")

    # 启动异步事件循环
    uasyncio.run(main())
    
def main():
    # ap = start_ap()  # 启动AP模式
    # start_web_server()  # 启动简单的Web服务器，等待配网信息
    # wlan = wifimgr.get_connection()
    # if wlan is None:
    #     print("Could not initialize the network connection.")
    #     while True:
    #         print('connect to wifi OK',wlan)
    #         pass  # you shall not pass :D
    # # Main Code goes here, wlan is a working network.WLAN(STA_IF) instance.
    # print("ESP OK")
    # connect_wifi_certain()#直连指定wifi
    # get_data_from_rx()  # 接收数据
    # tcp_client()
    uart = machine.UART(1, baudrate=115200, tx=1, rx=3)
    get_data_from_uart2tcp(uart)
if __name__ == '__main__':
    main()
    # start = mpyconnect()
    # start.connect()
    # while True:
    #
    #     if main():
    #         LED_breatheblink()  # 配网成功后，呼吸灯模式
    #     else:
    #         LED.off()  # 配网失败后，熄灭LED


#模拟链表删除 p-q-r 删除q
# p.next = q.next
# q.next = None
# p 和r之间插入q
# q.next = p.next
# p.next = q