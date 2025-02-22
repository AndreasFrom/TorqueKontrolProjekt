import tkinter as tk
from tkinter import messagebox
import socket
import threading
import csv
from datetime import datetime

# TCP Configuration
TCP_IP = "192.168.93.146"  # Replace with the Arduino's IP address
TCP_PORT = 4242            # Must match the Arduino's TCP port

# Global variables
logging = False
sock = None
csv_file = None
csv_writer = None

def create_csv_file():
    """Create a CSV file with a timestamped filename."""
    global csv_file, csv_writer
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")  # Format: YYYYMMDD_HHMMSS
    filename = f"sensor_data_{timestamp}.csv"
    csv_file = open(filename, mode="w", newline="")
    csv_writer = csv.writer(csv_file)
    # Write the header row
    csv_writer.writerow(["Timestamp", "Sensor Value"])

def close_csv_file():
    """Close the CSV file."""
    global csv_file
    if csv_file:
        csv_file.close()

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
    create_csv_file()  # Create a new CSV file
    send_command("START")
    threading.Thread(target=receive_data, daemon=True).start()

def stop_logging():
    """Stop logging by sending the STOP command."""
    global logging
    logging = False
    send_command("STOP")
    close_csv_file()  # Close the CSV file

def receive_data():
    """Receive sensor data from the Arduino and log it to the CSV file."""
    global sock, logging, csv_writer
    buffer = ""  # Buffer to store incomplete data
    while logging:
        try:
            data = sock.recv(1024).decode()
            if data:
                buffer += data  # Append incoming data to the buffer

                # Process complete lines
                while "\n" in buffer:
                    line, buffer = buffer.split("\n", 1)  # Split at the first newline
                    line = line.strip()  # Remove leading/trailing whitespace

                    # Parse the sensor value
                    if line.startswith("SENSOR:"):
                        sensor_value = line.split(":")[1]
                        # Get current time with milliseconds
                        now = datetime.now()
                        timestamp = now.strftime("%Y-%m-%d %H:%M:%S") + f".{now.microsecond // 1000:03d}"  # Add milliseconds

                        # Display the data in the text box
                        text_box.insert(tk.END, f"{timestamp} - {sensor_value}\n")
                        text_box.see(tk.END)  # Scroll to the bottom

                        # Write the data to the CSV file
                        csv_writer.writerow([timestamp, sensor_value])
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

# Close the socket and CSV file when the program exits
if sock:
    sock.close()
close_csv_file()