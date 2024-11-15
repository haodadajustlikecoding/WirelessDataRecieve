from django.db import models

# Create your models here.
from django.db import models

class TcpData(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    device_id = models.CharField(max_length=100)
    data = models.TextField()

    def __str__(self):
        return f"Data from {self.device_id} received at {self.timestamp}"

class IoTDeviceData(models.Model):
    device_id = models.CharField(max_length=100)
    timestamp = models.DateTimeField()
    data = models.JSONField()

    def __str__(self):
        return f"Device {self.device_id} at {self.timestamp}"