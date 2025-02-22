import socket

# TCP Configuration
TCP_IP = "192.168.93.146"  # Replace with the Arduino's IP address
TCP_PORT = 4242            # Must match the Arduino's TCP port

# Create a TCP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to the Arduino
    sock.connect((TCP_IP, TCP_PORT))
    print("Connected to Arduino!")

    # Send a message to the Arduino
    message = "START\n"
    print(f"Sending message: {message.strip()}")
    sock.send(message.encode())

    # Wait for a response
    response = sock.recv(1024).decode()
    print(f"Received response: {response.strip()}")
except Exception as e:
    print(f"Error: {e}")
finally:
    # Close the socket
    sock.close()