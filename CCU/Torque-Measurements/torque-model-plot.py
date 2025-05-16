import matplotlib.pyplot as plt
import numpy as np

slope = 43.968
intercept = 82.551

v = np.linspace(0, 3.5, 100)
torque = slope * v + intercept

plt.figure(figsize=(7, 4))
plt.plot(v, torque, label=r'$\tau_{total} = 43.968\,v + 82.551$', color='blue')
plt.xlabel("Velocity (m/s)")
plt.ylabel("Torque (*10^-3 Nm)")
plt.title("Torque vs Velocity")
plt.xlim(0, 3)
plt.grid()
plt.legend()
plt.show()