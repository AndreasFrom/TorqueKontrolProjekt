import socket
import time
import struct

# NTP server settings
NTP_PORT = 123
LOCAL_PORT = 2390
NTP_SERVER = '162.159.200.123'
NTP_PACKET_SIZE = 48

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', LOCAL_PORT))

print(f"Listening for UDP packets on port {LOCAL_PORT}...")

def create_ntp_response():
    # NTP packet format
    ntp_packet = bytearray(NTP_PACKET_SIZE)
    
    # Set the first byte (LI, Version, Mode)
    ntp_packet[0] = 0b00100100  # LI = 0 (no warning), Version = 4, Mode = 4 (server)
    
    # Set the Stratum (1 = primary server)
    ntp_packet[1] = 1
    
    # Set the Polling Interval (6 = 64 seconds)
    ntp_packet[2] = 6
    
    # Set the Precision (0xEC = -20 in decimal, which is 2^-20 seconds)
    ntp_packet[3] = 0xEC
    
    # Set the Root Delay (0x0000)
    ntp_packet[4:8] = struct.pack('!I', 0)
    
    # Set the Root Dispersion (0x0000)
    ntp_packet[8:12] = struct.pack('!I', 0)
    
    # Set the Reference Identifier (0x00000000)
    ntp_packet[12:16] = struct.pack('!I', 0)
    
    # Set the Reference Timestamp (current time)
    ref_time = time.time()
    ntp_packet[16:24] = struct.pack('!Q', int(ref_time * 2**32))
    
    # Set the Originate Timestamp (same as the client's transmit timestamp)
    # This will be filled in when we receive the client's packet
    
    # Set the Receive Timestamp (current time)
    recv_time = time.time()
    ntp_packet[32:40] = struct.pack('!Q', int(recv_time * 2**32))
    
    # Set the Transmit Timestamp (current time)
    transmit_time = time.time()
    ntp_packet[40:48] = struct.pack('!Q', int(transmit_time * 2**32))
    
    return ntp_packet

while True:
    # Receive data from the client
    data, addr = sock.recvfrom(NTP_PACKET_SIZE)
    print(f"Received NTP request from {addr}")
    
    # Create the NTP response packet
    ntp_response = create_ntp_response()
    
    # Copy the client's transmit timestamp to the originate timestamp in the response
    ntp_response[24:32] = data[40:48]
    
    # Send the NTP response back to the client
    sock.sendto(ntp_response, addr)
    print(f"Sent NTP response to {addr}")