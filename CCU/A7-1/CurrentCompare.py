import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import glob
import os

# Define folder patterns and labels
folder_patterns = [
    ('A7-1/v0.5-Speed/v05-S-*.csv', 'Speed'),
    ('A7-1/v0.5-ICOSpeed/v05-IS-*.csv', 'ICO Speed'),
    ('A7-1/v0.5-Torque/v05-T-*.csv', 'Torque'),
    ('A7-1/v0.5-ICOTorque/v05-IT-*.csv', 'ICO Torque')
]

current_columns = ['MU0current', 'MU1current', 'MU2current', 'MU3current']
x_labels = []
y_values = []
folder_stats = {}  # To store stats for each folder

for pattern, label in folder_patterns:
    file_list = glob.glob(pattern)[:5]
    if not file_list:
        print(f"No files found for {label}")
        continue

    currents = []
    for file in file_list:
        try:
            df = pd.read_csv(file, skiprows=range(1,500))
            df.columns = df.columns.str.strip()
            if not all(col in df.columns for col in current_columns):
                print(f"{os.path.basename(file)}: missing required columns, skipping.")
                continue
            avg_current = df[current_columns].mean().mean()
            x_labels.append(label)
            y_values.append(avg_current)
            currents.append(avg_current)
            print(f"{label} - {os.path.basename(file)}: Average current = {avg_current:.3f} A")
        except Exception as e:
            print(f"Error processing {file}: {e}")

    if currents:
        folder_stats[label] = {
            'max': np.max(currents),
            'min': np.min(currents),
            'mean': np.mean(currents),
            'std': np.std(currents)
        }

# Scatter plot: 5 points per folder label
plt.figure(figsize=(8, 5))
plt.scatter(x_labels, y_values, color='blue', s=100, alpha=0.7)
plt.title('Average Current Vs control modes v=0.5m/s')
plt.xlabel('Control modes')
plt.ylabel('Average Current (A)')
plt.grid(axis='y')
plt.ylim(0.20,0.40)

# Add info box for each folder
for i, label in enumerate(['Speed', 'ICO Speed', 'Torque', 'ICO Torque']):
    if label in folder_stats:
        stats = folder_stats[label]
        # Find the y-position for the box (above the max point for this label)
        y_pos = stats['max'] + 0.01
        textstr = (f"max: {stats['max']:.3f}\n"
                   f"min: {stats['min']:.3f}\n"
                   f"mean: {stats['mean']:.3f}\n"
                   f"std: {stats['std']:.3f}")
        plt.annotate(
            textstr,
            xy=(label, y_pos),
            xytext=(8, 0),
            textcoords='offset points',
            ha='center',
            va='bottom',
            bbox=dict(boxstyle='round,pad=0.3', fc='white', ec='gray', alpha=0.8),
            fontsize=9
        )

#plt.tight_layout()
plt.show()