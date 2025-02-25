import tkinter as tk
from tkinter import messagebox
import socket
import threading
import csv
from datetime import datetime

# TCP Configuration
TCP_IP = "192.168.93.231"  # Replace with the Arduino's IP address (Arduino prints ip in terminal on boot)
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
                        # Remove the "SENSOR:" prefix
                        sensor_data = line[len("SENSOR:"):].strip()

                        # Split into magnetometer, gyroscope, and accelerometer data
                        parts = sensor_data.split(" | ")
                        if len(parts) == 3:
                            omagn_data = parts[0].replace("Omagn: ", "").split(", ")
                            ogyro_data = parts[1].replace("Ogyro: ", "").split(", ")
                            oaccel_data = parts[2].replace("Oaccel: ", "").split(", ")

                            # Ensure we have exactly 3 values for each sensor
                            if len(omagn_data) == 3 and len(ogyro_data) == 3 and len(oaccel_data) == 3:
                                # Get current time with milliseconds
                                now = datetime.now()
                                timestamp = now.strftime("%Y-%m-%d %H:%M:%S") + f".{now.microsecond // 1000:03d}"

                                # Prepare the data for logging
                                sensor_values = {
                                    "timestamp": timestamp,
                                    "Omagn_x": float(omagn_data[0]),
                                    "Omagn_y": float(omagn_data[1]),
                                    "Omagn_z": float(omagn_data[2]),
                                    "Ogyro_x": float(ogyro_data[0]),
                                    "Ogyro_y": float(ogyro_data[1]),
                                    "Ogyro_z": float(ogyro_data[2]),
                                    "Oaccel_x": float(oaccel_data[0]),
                                    "Oaccel_y": float(oaccel_data[1]),
                                    "Oaccel_z": float(oaccel_data[2]),
                                }

                                # Display the data in the text box
                                text_box.insert(tk.END, f"{timestamp} - {sensor_values}\n")
                                text_box.see(tk.END)  # Scroll to the bottom

                                # Write the data to the CSV file
                                csv_writer.writerow([
                                    sensor_values["timestamp"],
                                    sensor_values["Omagn_x"], sensor_values["Omagn_y"], sensor_values["Omagn_z"],
                                    sensor_values["Ogyro_x"], sensor_values["Ogyro_y"], sensor_values["Ogyro_z"],
                                    sensor_values["Oaccel_x"], sensor_values["Oaccel_y"], sensor_values["Oaccel_z"],
                                ])
                            else:
                                print("Error: Invalid sensor data format")
                        else:
                            print("Error: Incomplete sensor data")
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