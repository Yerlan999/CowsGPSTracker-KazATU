from channels.routing import ProtocolTypeRouter, URLRouter
from django.urls import path
from chat.consumers import ChatConsumer

application = ProtocolTypeRouter({
    'websocket': URLRouter([
        path('ws/', ChatConsumer.as_asgi()),
    ])
})
