#!/usr/bin/env python
# -*- coding: UTF-8 -*-
'''
@Project ：WirelessDataRecieve
@File    ：start_server.py
@IDE     ：PyCharm 
@Author  ：FDU_WICAS_HS
@Email   ：shuai.hao@iclegend.com
@Date    ：2024/11/12 18:55
@ReadMe  ：  
'''
import os
import urllib

# import scipy.io as sio
import matplotlib.pyplot as plt
from pylab import mpl
# python manage.py start_server  8000 --host 127.0.0.1

mpl.rcParams['font.sans-serif'] = ['SimHei']
plt.rcParams['axes.unicode_minus'] = False  # 步骤二（解决坐标轴负数的负号显示问题）
import socket
from django.core.management.base import BaseCommand
from datareceiver.models import TcpData




# class Command(BaseCommand):
#     help = 'Starts a TCP server to receive data and save it in the database'
#
#     def add_arguments(self, parser):
#         parser.add_argument('port', type=int, help='The port on which the server will listen')
#         parser.add_argument('--host', default='127.0.0.1', help='The host IP address to bind to (default: 0.0.0.0)')
#
#     def handle(self, *args, **options):
#         host = options['host']  # 使用 --host 参数指定的IP地址，默认为 0.0.0.0
#         port = options['port']
#
#         with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#             s.bind((host, port))
#             s.listen()
#             self.stdout.write(self.style.SUCCESS(f'Server started on {host}:{port}'))
#
#             while True: # 监听端口，并接收数据 进行处理
#                 conn, addr = s.accept()
#                 with conn:
#                     self.stdout.write(self.style.SUCCESS(f'Connected by {addr}'))
#                     while True:
#                         data = conn.recv(1024)
#                         if not data:
#                             break
#                         TcpData.objects.create(data=data.decode()) # 接收数据 保存到data中，然后下面进行输出
#                         self.stdout.write(self.style.SUCCESS(f'Received and saved data: {data.decode()}'))




class Command(BaseCommand):
    help = 'Starts a TCP server to receive data and save it in the database'

    def add_arguments(self, parser):
        parser.add_argument('port', type=int, help='The port on which the server will listen')
        parser.add_argument('--host', default='0.0.0.0', help='The host IP address to bind to (default: 0.0.0.0)')

    def handle(self, *args, **options):
        host = options['host']  # 使用 --host 参数指定的IP地址，默认为 0.0.0.0
        port = options['port']

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((host, port))
            s.listen()
            self.stdout.write(self.style.SUCCESS(f'Server started on {host}:{port}'))

            while True:  # 监听端口，并接收数据进行处理
                conn, addr = s.accept()
                with conn:
                    self.stdout.write(self.style.SUCCESS(f'Connected by {addr}'))
                    while True:
                        data = conn.recv(1024)
                        if not data:
                            break

                        # 解码接收到的数据并解析
                        decoded_data = data.decode()  # 解码数据
                        self.stdout.write(self.style.SUCCESS(f'Received raw data: {decoded_data}'))

                        try:
                            parsed_data = urllib.parse.parse_qs(decoded_data)  # 解析查询字符串
                        except Exception as e:
                            self.stdout.write(self.style.WARNING(f'Error parsing data: {e}'))
                            continue

                        # 提取 device_id, timestamp 和 data
                        device_id = parsed_data.get('device_id', [''])[0]
                        timestamp = parsed_data.get('timestamp', [''])[0]
                        data_str = parsed_data.get('data', [''])[0]

                        self.stdout.write(f'Parsed data: device_id={device_id}, timestamp={timestamp}, data={data_str}')

                        # 数据验证
                        if device_id and timestamp and data_str:
                            # 如果数据有效，进行额外处理（如转换、清理、计算等）
                            processed_data = self.process_data(device_id, timestamp, data_str)

                            # 保存数据到数据库
                            TcpData.objects.create(device_id=device_id, timestamp=timestamp, data=processed_data)
                            self.stdout.write(self.style.SUCCESS(f'Received and saved data: {device_id} - {processed_data}'))
                        else:
                            self.stdout.write(self.style.WARNING(f'Invalid data received: {decoded_data}'))

    def process_data(self, device_id, timestamp, data_str):
        """
        对接收到的数据进行处理（如转换、清理等）
        """
        # 你可以在这里做一些数据处理，比如转换数据格式、去除空格等
        # 例如：将数据字符串转化为某种格式或进行数据清洗
        processed_data = data_str.strip().replace(' ', ',')  # 示例处理：去除空格并替换为逗号
        return processed_data

