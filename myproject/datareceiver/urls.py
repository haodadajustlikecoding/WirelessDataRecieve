#!/usr/bin/env python
# -*- coding: UTF-8 -*-
'''
@Project ：Django_Server
@File    ：urls.py
@IDE     ：PyCharm 
@Author  ：FDU_WICAS_HS
@Email   ：shuai.hao@iclegend.com
@Date    ：2024/11/13 21:18
@ReadMe  ：  
'''
import numpy as np
import os, time, sys
import scipy.io as sio
import matplotlib.pyplot as plt
from pylab import mpl

mpl.rcParams['font.sans-serif'] = ['SimHei']
plt.rcParams['axes.unicode_minus'] = False  # 步骤二（解决坐标轴负数的负号显示问题）
from django.urls import path
from .views import  submit_data

urlpatterns = [
    path('submit_data', submit_data, name='submit_data'),
]