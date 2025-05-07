import tkinter as tk
from tkinter import messagebox, StringVar, OptionMenu
import socket
import threading
import csv
from datetime import datetime

# TCP Configuration
#TCP_IP = "192.168.93.231"  # Replace with the Arduino's IP address (Arduino prints ip in terminal on boot)
TCP_IP = "192.168.137.69"
TCP_PORT = 4242            # Must match the Arduino's TCP port

# Global variables
logging = False
sock = None
csv_file = None
csv_writer = None

debug = False

def send_command(command):
    """Send a command to the Arduino and wait for acknowledgments and follow-ups."""
    global sock
    try:
        if sock is None:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5)  # Set timeout
            sock.connect((TCP_IP, TCP_PORT))

        sock.send(f"{command}\n".encode())
        print(f"Sent command: {command}")
        text_box.insert(tk.END, f"Sent command: {command}\n")
        text_box.see(tk.END)  # Scroll to the bottom

        # Wait for initial acknowledgment
        ack = sock.recv(1024).decode().strip()
        if ack.startswith("ACK:"):
            print(f"Acknowledgment received: {ack}")
            text_box.insert(tk.END, f"Acknowledgment received: {ack}\n")
        elif ack.startswith("ERROR:"):
            print(f"Error from Arduino: {ack}")
            text_box.insert(tk.END, f"Error from Arduino: {ack}\n")
            return  # Stop if there's an error

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


# Function to send PID parameters and setpoint
def send_pid_setpoint():
    """Send PID parameters and setpoint to the Arduino."""
    try:
        kp = float(entry_kp.get())
        ki = float(entry_ki.get())
        kd = float(entry_kd.get())
        setpoint = float(entry_setpoint.get())
        mode = float(entry_mode.get())
        send_command(f"PID:{kp},{ki},{kd},{setpoint},{mode}")
    except ValueError:
        print("Error: Invalid input for PID parameters or setpoint")


def send_ico():
    """Send ICO command to the Arduino."""
    try:
        omega0 = float(entry_omega0.get())
        omega1 = float(entry_omega1.get())
        eta = float(entry_eta.get())
        send_command(f"ICO:{omega0},{omega1},{eta}")
    except ValueError:
        print("Error: Invalid input for ICO parameters")

# Function to send only the setpoint
def send_setpoint_only():
    """Send only the setpoint to the Arduino (keep PID parameters unchanged)."""
    try:
        setpoint = float(entry_setpoint.get())
        send_command(f"SETPOINT:{setpoint}")
    except ValueError:
        print("Error: Invalid input for setpoint")



def receive_data():
    """Receive sensor data from the Arduino and log it to the CSV file."""
    global sock, logging, csv_writer
    buffer = ""  # Buffer to store incomplete data
    while logging:
        try:
            data = sock.recv(1024).decode()
            if data:
                buffer += data  # Append incoming data to the buffer

                """ # Process complete lines
                while "\n" in buffer:
                    line, buffer = buffer.split("\n", 1)  # Split at the first newline
                    line = line.strip()  # Remove leading/trailing whitespace
                    if debug: print(f"Received line: {line}")  # Debug print

                    if line.startswith("TIME:"):
                        time_block, imu_block = line.split(" | ", 1) # Split at the first " | " (time, IMU)
                        time_data = time_block[len("TIME:"):].strip() # Remove the "TIME:" prefix
                        if debug: print(f"Time data: {time_data}")  # Debug print

                        if imu_block.startswith("IMU:"):
                            imu_data = imu_block[len("IMU:"):].strip() # Remove the "IMU:" prefix
                            oaccel_block, ogyro_block = imu_data.split(" | ", 1) # Split at the first " | " (Oaccel, Ogyro)
                            oaccel_data = oaccel_block.replace("Oaccel: ", "").split(", ")
                            ogyro_data = ogyro_block.replace("Ogyro: ", "").split(", ")
                            if debug: print(f"IMU data: {imu_data}")  # Debug print
                            if debug: print(f"Oaccel data: {oaccel_data}")  # Debug print
                            if debug: print(f"Ogyro data: {ogyro_data}")  # Debug print

                            if len(oaccel_data) == 2 and len(ogyro_data) == 1:
                                sensor_values = {
                                    "Timestamp": float(time_data),
                                    "Oaccel_x": float(oaccel_data[0]),
                                    "Oaccel_y": float(oaccel_data[1]),
                                    "Ogyro_z": float(ogyro_data[0]),
                                }

                                # Display the data in the text box
                                text_box.insert(tk.END, f"{time_data} Logged data\n")
                                text_box.see(tk.END)  # Scroll to the bottom

                                csv_writer.writerow([
                                    sensor_values["Timestamp"],
                                    sensor_values["Oaccel_x"], sensor_values["Oaccel_y"], sensor_values["Ogyro_z"],
                                ])
                            else:
                                print("Error: Invalid sensor data format") """
                        
        except socket.timeout:
            continue
        except Exception as e:
            print(f"Error receiving data: {e}")
            break

# Create the main UI window
root = tk.Tk()
root.title("Arduino Sensor Logger")

# Movement categories
other_commands = ["Select Movement"]
green_commands = ["Circle"]
red_commands = ["Straight", "Straight with Turn", "Turn Left 90°", "Turn Right 90°", "Turn Left 45°", "Turn Right 45°", "U-Turn"]

options = green_commands + red_commands

# Variable for dropdown
clicked = StringVar()
clicked.set("Select Movement")

# Function to handle dropdown selection
def handle_selection(choice):
    """Send command based on dropdown selection."""
    commands = {
        "Select Movement": None,
        "Circle": "CIRCLE",
        "Straight": "STRAIGHT",
        "Straight with Turn": "STRAIGHT_TURN",
        "Turn Left 90°": "TURN_LEFT_90",
        "Turn Right 90°": "TURN_RIGHT_90",
        "Turn Left 45°": "TURN_LEFT_45",
        "Turn Right 45°": "TURN_RIGHT_45",
        "U-Turn": "U_TURN"
    }
    command = commands.get(choice)
    if command:
        send_command(command)

# Create Dropdown menu
drop = OptionMenu(root, clicked, *options, command=handle_selection)
drop.grid(row=0, column=0, padx=10, pady=5, sticky="w")

# Set colors for dropdown menu options
def set_color(menu):
    for i, option in enumerate(options):
        color = "green" if option in green_commands else "red" if option in red_commands else "gray"
        menu.entryconfig(i, foreground=color)

menu = root.nametowidget(drop.menuname)
root.after(100, lambda: set_color(menu))

# UI Elements for PID and Setpoint
label_kp = tk.Label(root, text="Kp:")
label_kp.grid(row=1, column=0, padx=10, pady=5, sticky="w")
entry_kp = tk.Entry(root)
entry_kp.grid(row=1, column=1, padx=10, pady=5, sticky="w")
entry_kp.insert(0, "1.32")

label_ki = tk.Label(root, text="Ki:")
label_ki.grid(row=2, column=0, padx=10, pady=5, sticky="w")
entry_ki = tk.Entry(root)
entry_ki.grid(row=2, column=1, padx=10, pady=5, sticky="w")
entry_ki.insert(0, "52.8")

label_kd = tk.Label(root, text="Kd:")
label_kd.grid(row=3, column=0, padx=10, pady=5, sticky="w")
entry_kd = tk.Entry(root)
entry_kd.grid(row=3, column=1, padx=10, pady=5, sticky="w")
entry_kd.insert(0, "0.008")

label_setpoint = tk.Label(root, text="Setpoint:")
label_setpoint.grid(row=4, column=0, padx=10, pady=5, sticky="w")
entry_setpoint = tk.Entry(root)
entry_setpoint.grid(row=4, column=1, padx=10, pady=5, sticky="w")
entry_setpoint.insert(0, "0.5")

label_mode = tk.Label(root, text="Mode:")
label_mode.grid(row=5, column=0, padx=10, pady=5, sticky="w")
entry_mode = tk.Entry(root)
entry_mode.grid(row=5, column=1, padx=10, pady=5, sticky="w")
entry_mode.insert(0, "1")

button_send_pid = tk.Button(root, text="Send PID & Setpoint", command=send_pid_setpoint)
button_send_pid.grid(row=6, column=0, columnspan=2, padx=10, pady=5, sticky="w")

button_send_setpoint = tk.Button(root, text="Send Setpoint Only", command=send_setpoint_only)
button_send_setpoint.grid(row=6, column=1, columnspan=2, padx=10, pady=10, sticky="w")

# UI elements omega 0 and omega 1
label_omega0 = tk.Label(root, text="Omega 0:")
label_omega0.grid(row=7, column=0, padx=10, pady=5, sticky="w")
entry_omega0 = tk.Entry(root)
entry_omega0.grid(row=7, column=1, padx=10, pady=5, sticky="w")
entry_omega0.insert(0, "0.0025")

label_omega1 = tk.Label(root, text="Omega 1:")
label_omega1.grid(row=8, column=0, padx=10, pady=5, sticky="w")
entry_omega1 = tk.Entry(root)
entry_omega1.grid(row=8, column=1, padx=10, pady=5, sticky="w")
entry_omega1.insert(0, "0.0")

label_eta = tk.Label(root, text="Learning rate:")
label_eta.grid(row=9, column=0, padx=10, pady=5, sticky="w")
entry_eta = tk.Entry(root)
entry_eta.grid(row=9, column=1, padx=10, pady=5, sticky="w")
entry_eta.insert(0, "0.00000001")

# button to send ICO command
button_send_ico = tk.Button(root, text="Send ICO", command=send_ico)
button_send_ico.grid(row=10, column=1, padx=10, pady=5, sticky="w")

# UI Elements for logging
button_start = tk.Button(root, text="Start", command=start_logging)
button_start.grid(row=11, column=0, padx=10, pady=5, sticky="w")

button_stop = tk.Button(root, text="Stop", command=stop_logging)
button_stop.grid(row=12, column=0, padx=10, pady=5, sticky="w")



# Text box for logging
text_box = tk.Text(root, height=10, width=50)
text_box.grid(row=0, column=2, rowspan=8, padx=10, pady=5, sticky="nsew")  

# Make the text box expand properly when resizing
root.grid_columnconfigure(2, weight=1)
root.grid_rowconfigure(0, weight=1)

root.mainloop()
