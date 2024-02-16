import threading
import websocket

def on_message(ws, message):
    print("Received: ", message)

# Connect to the WebSocket server
IP = "192.168.54.6:80"
ws_client = websocket.WebSocketApp("ws://" + IP, on_message=on_message)
ws_client_thread = threading.Thread(target=ws_client.run_forever)
ws_client_thread.start()

def send():
    global ws_client  # Make ws_client a global variable
    try:
        while True:
            # Prompt the user to enter a message
            mssg = input("Enter something: ")

            # Send the message to the server
            ws_client.send(mssg)

            # Print the sent message
            print("Sent via WebSocket: " + mssg)
    except KeyboardInterrupt:
        print("\nKeyboardInterrupt: Closing WebSocket connection.")
        ws_client.close()

if __name__ == "__main__":
    # Start the message sender in the main thread
    send()
