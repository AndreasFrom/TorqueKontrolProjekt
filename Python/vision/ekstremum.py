import numpy as np
import cv2
import os
import matplotlib.pyplot as plt
import pandas as pd
import scipy.signal as signal

plot_peaks = False
plot_distances = True

plot_peaks = True
plot_distances = True

def print_extreme(csv_path):
    # Load data
    df = pd.read_csv(csv_path)

    # Extract columns
    fps = df["fps"].iloc[0]  # assuming FPS is constant
    frames = df["Frame"]
    x_positions = df["X Position (m)"]
    y_positions = df["Y Position (m)"]
    seconds = frames / fps

    # Peak and low detection (tweak params for your data)
    x_peaks, _ = signal.find_peaks(
        x_positions, distance=5, prominence=0.3, width=2
    )
    x_lows, _ = signal.find_peaks(
        -x_positions, distance=3, prominence=0.1, width=1
    )

    # Print low point information
    for i in range(len(x_lows)):
        print(f"Low {i + 1}:")
        print(f"  Time: {seconds[x_lows[i]]:.2f} s")
        print(f"  X Position: {x_positions[x_lows[i]]:.2f} m")
        print(f"  Y Position: {y_positions[x_lows[i]]:.2f} m")

    # Plot peaks and lows if enabled
    if plot_peaks:
        plt.figure(figsize=(10, 6))
        plt.plot(seconds, x_positions, label='X Position (m)', color='blue')
        plt.plot(seconds[x_peaks], x_positions[x_peaks], "x", label='Peaks', color='red')
        plt.plot(seconds[x_lows], x_positions[x_lows], "o", label='Lows', color='green')
        plt.xlabel('Time (s)')
        plt.ylabel('X Position (m)')
        plt.title('X Position with Peaks and Lows')
        plt.legend()
        plt.grid(True)
        plt.show()

    # Calculate 2D Euclidean distances
    distances = []

    # Determine if the first point is a peak or a low
    start_index = 0 if x_peaks[0] < x_lows[0] else 1

    for i in range(min(len(x_peaks), len(x_lows)) - 1):
        if start_index == 0:
            # Peak → Low
            p_idx = x_peaks[i]
            l_idx = x_lows[i]
            d1 = np.linalg.norm([
                x_positions[l_idx] - x_positions[p_idx],
                y_positions[l_idx] - y_positions[p_idx]
            ])
            # Low → Next Peak
            np_idx = x_peaks[i + 1]
            d2 = np.linalg.norm([
                x_positions[np_idx] - x_positions[l_idx],
                y_positions[np_idx] - y_positions[l_idx]
            ])
        else:
            # Low → Peak
            l_idx = x_lows[i]
            p_idx = x_peaks[i]
            d1 = np.linalg.norm([
                x_positions[p_idx] - x_positions[l_idx],
                y_positions[p_idx] - y_positions[l_idx]
            ])
            # Peak → Next Low
            nl_idx = x_lows[i + 1]
            d2 = np.linalg.norm([
                x_positions[nl_idx] - x_positions[p_idx],
                y_positions[nl_idx] - y_positions[p_idx]
            ])
        distances.append((d1, d2))
        start_index = 1 - start_index  # Alternate for next pair

    # Print distances
    for i, (d1, d2) in enumerate(distances):
        print(f"\nSegment {i + 1}:")
        print(f"  First segment distance: {d1:.3f} m")
        print(f"  Second segment distance: {d2:.3f} m")

    # Plot distances if enabled
    if plot_distances:
        plt.figure(figsize=(10, 6))
        plt.plot(seconds, x_positions, label='X Position (m)', color='blue')
        plt.plot(seconds[x_peaks], x_positions[x_peaks], "x", label='Peaks', color='red')
        plt.plot(seconds[x_lows], x_positions[x_lows], "o", label='Lows', color='green')
        for i in range(min(len(x_peaks), len(x_lows)) - 1):
            try:
                if i < len(distances):
                    d1, d2 = distances[i]
                    if start_index == 0:
                        # Backtrack: peak → low → next peak
                        p_idx = x_peaks[i]
                        l_idx = x_lows[i]
                        np_idx = x_peaks[i + 1]
                        plt.plot([seconds[p_idx], seconds[l_idx]], [x_positions[p_idx], x_positions[l_idx]], 'r--')
                        plt.plot([seconds[l_idx], seconds[np_idx]], [x_positions[l_idx], x_positions[np_idx]], 'g--')
                    else:
                        l_idx = x_lows[i]
                        p_idx = x_peaks[i]
                        nl_idx = x_lows[i + 1]
                        plt.plot([seconds[l_idx], seconds[p_idx]], [x_positions[l_idx], x_positions[p_idx]], 'g--')
                        plt.plot([seconds[p_idx], seconds[nl_idx]], [x_positions[p_idx], x_positions[nl_idx]], 'r--')
            except IndexError:
                continue
        plt.xlabel('Time (s)')
        plt.ylabel('X Position (m)')
        plt.title('X Position with Segment Distances')
        plt.legend()
        plt.grid(True)
        plt.show()

    

if __name__ == "__main__":
    input_folder = os.path.join(os.path.dirname(__file__), 'output_files_repaired')
    #output_folder = os.path.join(os.path.dirname(__file__), 'output_plots')
    #sos.makedirs(output_folder, exist_ok=True)
    
    for file_name in os.listdir(input_folder):
        if file_name.endswith('.csv'):
            csv_file = os.path.join(input_folder, file_name)
            #plot_output = os.path.join(output_folder, file_name.replace('.csv', '.png'))
            print(f"Processing {csv_file}...")

            print_extreme(csv_file)
