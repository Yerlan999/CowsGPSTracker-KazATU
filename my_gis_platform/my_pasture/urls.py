from django.urls import path
from . import views

urlpatterns = [
    path('', views.pasture, name='pasture'),
    path('about/', views.about, name='about'),
    path('available_dates/', views.available_dates, name='available_dates'),
    path('available_dates/<int:pk>/', views.date_detail, name='date_detail'),
]
