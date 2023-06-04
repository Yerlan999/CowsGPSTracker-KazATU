from django.urls import path
from . import views

urlpatterns = [
    path('pasture/', views.pasture, name='pasture'),
    path('about/', views.about, name='about'),
    path('available_dates/', views.available_dates, name='available_dates'),
]
