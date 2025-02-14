import serial
import time
import pandas as pd
import matplotlib.pyplot as plt

# Serial port setup (adjust as needed)
ser = serial.Serial('COM3', 115200, timeout=1)  # Change 'COM3' to your Arduino port
time.sleep(2)  # Give time for serial connection to establish
ser.flush()

# Create an empty list to store data
data_list = []
start_time = time.time()  # Record the start time

# Run for 20 seconds
while time.time() - start_time < 3:
    line = ser.readline().decode('utf-8', errors='ignore').strip()  # Read and decode the serial line
    if line:
        try:
            sensor_time, speed, pwm_value = line.split(",")
            sensor_time = int(sensor_time)
            speed = float(speed)
            pwm_value = int(pwm_value)

            data_list.append([sensor_time, speed, pwm_value])

        except ValueError:
            print(f"Warning: Skipping malformed line: {line}")  # Debugging for incorrect data

# Close serial connection
ser.close()
print("Serial connection closed.")

# Save data to CSV
df = pd.DataFrame(data_list, columns=["sensor_time", "speed", "pwm_value"])
df.to_csv("motor_data.csv", index=False)
print("Data saved to motor_data.csv")

# Save the final graph
plt.figure(figsize=(8, 5))
plt.plot(df["speed"], label="Speed (RPM)", color='b')
plt.plot(df["pwm_value"], label="PWM Value", color='g')

plt.title("Motor Speed & PID Control")
plt.xlabel("Time (Samples)")
plt.ylabel("Value")
plt.legend(loc="upper right")
plt.grid()

plt.savefig("motor_graph.png")  # Save the plot as an image
print("Graph saved as motor_graph.png")
