from django.shortcuts import render

# Create your views here.
import os
import csv
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from django.utils.dateparse import parse_datetime

# 保存数据的目录
save_dir = "data_files"

from django.http import JsonResponse

def submit_data(request):
    device_id = request.GET.get('device_id')
    timestamp = request.GET.get('timestamp')
    data = request.GET.get('data')

    if device_id and timestamp and data:
        return JsonResponse({
            'status': 'success',
            'device_id': device_id,
            'timestamp': timestamp,
            'data': data
        })
    else:
        return JsonResponse({'status': 'error', 'message': 'Missing parameters'}, status=400)
