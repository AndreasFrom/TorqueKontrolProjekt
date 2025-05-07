import matplotlib.pyplot as plt
from scipy import stats

# Data 
x = [0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0]
y = [0.903,1.080,1.068,1.127,1.197,1.177,1.244,1.267]

# Regressor
slope, intercept, r, p, std_err = stats.linregress(x, y)

def myfunc(x):
  return slope * x + intercept

mymodel = list(map(myfunc, x))

plt.xlabel("Velocity")
plt.ylabel("Current")
plt.grid()

equation = f"y = {slope:.4f}x + {intercept:.4f}"
plt.text(0.5, max(y) - 0.05, equation, color='Blue', fontsize=12)
plt.legend()

plt.scatter(x, y)
plt.plot(x, mymodel)
plt.show()