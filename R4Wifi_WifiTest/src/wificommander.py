import tkinter as tk
from tkinter import messagebox
import socket
import threading

# TCP Configuration
TCP_IP = "192.168.93.146"  # Replace with the Arduino's IP address
TCP_PORT = 4242            # Must match the Arduino's TCP port

# Global variables
logging = False
sock = None

def send_command(command):
    """Send a command to the Arduino."""
    global sock
    try:
        if sock is None:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5)  # Set a timeout for receiving data
            sock.connect((TCP_IP, TCP_PORT))

        sock.send(f"{command}\n".encode())
        print(f"Sent command: {command}")
    except Exception as e:
        messagebox.showerror("Error", f"Failed to send {command} command: {e}")

def start_logging():
    """Start logging by sending the START command."""
    global logging
    logging = True
    send_command("START")
    threading.Thread(target=receive_data, daemon=True).start()

def stop_logging():
    """Stop logging by sending the STOP command."""
    global logging
    logging = False
    send_command("STOP")

def receive_data():
    """Receive sensor data from the Arduino."""
    global sock, logging
    while logging:
        try:
            data = sock.recv(1024).decode()
            if data:
                text_box.insert(tk.END, data)
                text_box.see(tk.END)  # Scroll to the bottom
        except socket.timeout:
            continue
        except Exception as e:
            print(f"Error receiving data: {e}")
            break

# Create the main UI window
root = tk.Tk()
root.title("Arduino Sensor Logger")

# Create and place UI elements
button_start = tk.Button(root, text="Start Logging", command=start_logging)
button_start.pack(padx=20, pady=10)

button_stop = tk.Button(root, text="Stop Logging", command=stop_logging)
button_stop.pack(padx=20, pady=10)

text_box = tk.Text(root, height=10, width=50)
text_box.pack(padx=20, pady=10)

# Run the UI
root.mainloop()

# Close the socket when the program exits
if sock:
    sock.close()