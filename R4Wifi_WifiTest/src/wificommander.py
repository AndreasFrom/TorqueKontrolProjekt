import tkinter as tk
from tkinter import messagebox
import socket
import threading

# TCP Configuration
TCP_IP = "192.168.93.146"  # Replace with the Arduino's IP address
TCP_PORT = 4242            # Must match the Arduino's TCP port

# Global variable to store received data
received_data = ""

def send_start_command():
    """Send the START command to the Arduino."""
    try:
        # Create a TCP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)  # Set a timeout for receiving data

        # Connect to the Arduino
        sock.connect((TCP_IP, TCP_PORT))

        # Send the START command
        sock.send("START\n".encode())

        # Wait for a response
        data = sock.recv(1024).decode()
        global received_data
        received_data = data.strip()

        # Update the UI with the received data
        text_box.insert(tk.END, received_data + "\n")
        text_box.see(tk.END)  # Scroll to the bottom

        # Close the socket
        sock.close()
    except Exception as e:
        messagebox.showerror("Error", f"Failed to send START command: {e}")

def start_logging():
    """Start a thread to send the START command."""
    threading.Thread(target=send_start_command, daemon=True).start()

# Create the main UI window
root = tk.Tk()
root.title("Arduino Sensor Logger")

# Create and place UI elements
button_start = tk.Button(root, text="Start Logging", command=start_logging)
button_start.pack(padx=20, pady=10)

text_box = tk.Text(root, height=10, width=50)
text_box.pack(padx=20, pady=10)

# Run the UI
root.mainloop()