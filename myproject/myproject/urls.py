"""
URL configuration for myproject project.

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/5.1/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path


from django.contrib import admin
from django.urls import path, include
from datareceiver.views import submit_data
urlpatterns = [
    path('admin/', admin.site.urls),
    path('submit_data', submit_data, name='submit_data'), # 必须要在根目录的urls.py中添加

    path('data/', include('datareceiver.urls')),  # 添加这个行来包含我们的数据处理应用的URL

]



