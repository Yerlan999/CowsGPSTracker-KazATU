from django.urls import path
from . import views

urlpatterns = [
    path('', views.dates_request, name='dates_request'),
    path('about/', views.about, name='about'),
    path('available_dates/', views.available_dates, name='available_dates'),
    path('available_dates/<int:pk>/', views.date_detail, name='date_detail'),
    path('ajax/', views.ajax_view, name='ajax'),
]
