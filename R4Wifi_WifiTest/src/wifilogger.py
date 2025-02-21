import socket

UDP_IP = "192.168.93.49"  # Use your actual WiFi IP
UDP_PORT = 4242  # Port to listen on

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for UDP packets on {UDP_IP}:{UDP_PORT}...")

while True:
    data, addr = sock.recvfrom(1024)  # Buffer size is 1024 bytes
    print(f"Received message: {data.decode()} from {addr}")

    # Send ACK back
    ack_message = "ACK".encode()
    sock.sendto(ack_message, addr)
    print("ACK sent.")
