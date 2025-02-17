import serial
import time
import pandas as pd
import matplotlib.pyplot as plt
import argparse

# Set up argument parsing
parser = argparse.ArgumentParser(description="Plot motor speed and PWM data.")
parser.add_argument("--target_rpm", type=float, required=True, help="Target RPM to mark on the plot")
args = parser.parse_args()

# Serial port setup (adjust as needed)
ser = serial.Serial('COM3', 115200, timeout=1)  # Change 'COM3' to your Arduino port
time.sleep(2)  # Give time for serial connection to establish
ser.flush()

# Create an empty list to store data
data_list = []
start_time = time.time()  # Record the start time

# Run for 2 seconds
while time.time() - start_time < 0.5:
    line = ser.readline().decode('utf-8', errors='ignore').strip()  # Read and decode the serial line
    if line:
        try:
            # Split the line into components
            values = line.split(",")
            if len(values) == 4:  # Ensure there are exactly 4 values
                time_stamp, sensor_time, speed, pwm_value = values
                time_stamp = float(time_stamp) / 1000.0  # Convert to seconds
                sensor_time = int(sensor_time)
                speed = float(speed)
                pwm_value = int(pwm_value)

                # Append data to the list
                data_list.append([time_stamp, sensor_time, speed, pwm_value])
            else:
                print(f"Warning: Skipping malformed line (incorrect number of values): {line}")
        except ValueError:
            print(f"Warning: Skipping malformed line (invalid data): {line}")  # Debugging for incorrect data

# Close serial connection
ser.close()
print("Serial connection closed.")

# Save data to CSV
df = pd.DataFrame(data_list, columns=["time_stamp", "sensor_time", "speed", "pwm_value"])
df.to_csv("motor_data.csv", index=False)
print("Data saved to motor_data.csv")

# Plot the data
plt.figure(figsize=(8, 5))
plt.plot(df["time_stamp"], df["speed"], label="Speed (RPM)", color='b')
plt.plot(df["time_stamp"], df["pwm_value"], label="PWM Value", color='g')

# Mark the target RPM
target_rpm = args.target_rpm
plt.axhline(y=target_rpm, color='r', linestyle='--', label=f"Target RPM ({target_rpm})")

# Mark points where speed equals target RPM
hits_target = df[df["speed"] == target_rpm]
if not hits_target.empty:
    plt.scatter(hits_target["time_stamp"], hits_target["speed"], color='r', label=f"Hit {target_rpm} RPM")

plt.title("Motor Speed and PWM Data on step response")
plt.xlabel("Time (seconds)")
plt.ylabel("Value")
plt.legend(loc="upper right")
plt.grid()

plt.savefig("motor_graph.png")  # Save the plot as an image
print("Graph saved as motor_graph.png")