import pandas as pd
import numpy as np
import scipy.integrate as integrate

# Load and prepare df
df = pd.read_csv('1088deg_turn.CSV')
df.columns = df.columns.str.strip()

# Convert timestamp to seconds
df["time_sec"] = (df["timestamp"] - df["timestamp"].min()) / 1000  

# Extract time and gyro_z values
time = df["time_sec"].values

# Extract Motor values M0 and M1 and convert to m/s isntead of rpm
M0_velocity = df["MU0value"].values * 0.068 * np.pi / 60  
M1_velocity = df["MU1value"].values * 0.068 * np.pi / 60

displacement_0 = integrate.trapezoid(M0_velocity, time)  # Integrate M0 to get displacement
displacement_1 = integrate.trapezoid(M1_velocity, time)  # Integrate M1 to get displacement


print("Displacement MO in distance: " + str(displacement_0))
print("Displacement M1 in distance: " + str(displacement_1))

# Gyro_z values are in degrees/sec
gyro_z = df["gyro_z"].values   # Convert to rad/sec
angular_velocity = integrate.cumulative_trapezoid(gyro_z, time, initial=0)
displacement_rotation = integrate.trapezoid(angular_velocity, time)

print(f"Total angular displacement in degrees: {displacement_rotation} degrees")