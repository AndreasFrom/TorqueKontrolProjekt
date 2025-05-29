import matplotlib.pyplot as plt
import pandas as pd
import os
from scipy.signal import savgol_filter
import numpy as np

def plot_marker_data(csv_path, output_plot_path):
    # Load data
    df = pd.read_csv(csv_path)
    
    # Extract columns
    fps = df["fps"]
    frames = df["Frame"]
    x_positions = df["X Position (m)"]
    y_positions = df["Y Position (m)"]
    distances = df["Distance from Circle (m)"]
    seconds = frames * 1 / fps

    # Smooth positions
    window_size = int(fps.iloc[0] * 1)
    if window_size % 2 == 0:
        window_size += 1  # window length must be odd for savgol_filter

    smoothed_x = savgol_filter(x_positions, window_length=window_size, polyorder=2)
    smoothed_y = savgol_filter(y_positions, window_length=window_size, polyorder=2)

    # Recalculate speed from smoothed positions
    dx = np.gradient(smoothed_x)
    dy = np.gradient(smoothed_y)
    smoothed_speed = np.sqrt(dx**2 + dy**2) * fps.iloc[0]

    # Create plots
    fig_pos_time, axes = plt.subplots(1, 1, figsize=(10, 8))
    
    # Plot X and Y positions
    axes.plot(seconds, x_positions, label='X Position', color='b', alpha=0.4)
    axes.plot(seconds, y_positions, label='Y Position', color='g', alpha=0.4)
    axes.plot(seconds, smoothed_x, label='Smoothed X', color='b')
    axes.plot(seconds, smoothed_y, label='Smoothed Y', color='g')
    axes.set_xlabel("s")
    axes.set_ylabel("Position (m)")
    axes.set_title("Marker 10 Position Over Time")
    axes.legend()
    axes.grid(which='both', linestyle='--', linewidth=0.5)
    
    fig_pos_time.tight_layout()
    fig_pos_time.savefig(output_plot_path.replace('.png', '_position.png'))
    plt.close(fig_pos_time)
    
    # Create speed plot
        # Create speed plot
    fig_speed, axes = plt.subplots(2, 1, figsize=(10, 4))

    # Compute original speed from raw positions
    raw_dx = np.gradient(x_positions)
    raw_dy = np.gradient(y_positions)
    raw_speed = np.sqrt(raw_dx**2 + raw_dy**2) * fps.iloc[0]

    # Plot raw and smoothed speed
    axes[0].plot(seconds, raw_speed, label='Raw Speed', color='purple', alpha=0.4)
    axes[0].plot(seconds, smoothed_speed, label='Smoothed Speed', color='black')
    axes[0].set_xlabel("s")
    axes[0].set_ylabel("Speed (m/s)")
    axes[0].set_title("Marker 10 Speed Over Time")
    axes[0].legend()
    axes[0].grid(which='both', linestyle='--', linewidth=0.5)
    axes[0].yaxis.set_major_locator(plt.MaxNLocator(nbins=15)) 
    axes[0].set_ylim(0, 1.5) 

    # Moving average of smoothed speed
    moving_avg_speed = pd.Series(smoothed_speed).rolling(window=window_size).mean()
    axes[1].plot(seconds, moving_avg_speed, label='Moving Average Speed', color='orange')
    axes[1].set_xlabel("s")
    axes[1].set_ylabel("Speed (m/s)")
    axes[1].set_title(f"Marker 10 Speed Moving Average (Window Size: {window_size})")
    axes[1].legend()
    axes[1].grid(which='both', linestyle='--', linewidth=0.5)
    axes[1].yaxis.set_major_locator(plt.MaxNLocator(nbins=15))
    axes[1].set_ylim(0, 1.5)

    # print median of moving average speed with name of file
    median_speed = moving_avg_speed.median()
    print(f"Median Speed for {os.path.basename(csv_path)}: {median_speed:.3f} m/s")
    
    fig_speed.tight_layout()
    fig_speed.savefig(output_plot_path.replace('.png', '_speed.png'))
    plt.close(fig_speed)

    
    # Plot positions
    plt.figure(figsize=(6, 6))  # Set figure size to ensure a square plot
    plt.plot(x_positions, y_positions, label='Position', color='green', alpha=0.5, linewidth=1)
    
    # Add a vector arrow to indicate the starting direction and location using the first few points
    # Define minimum movement threshold (in meters)
    min_movement = 0.1
    arrow_length = 0.05  # Set the desired arrow length (in meters)

    # Find the first point where movement exceeds the threshold
    start_index = 0
    for i in range(1, len(x_positions)):
        dx = x_positions.iloc[i] - x_positions.iloc[start_index]
        dy = y_positions.iloc[i] - y_positions.iloc[start_index]
        if (dx**2 + dy**2)**0.5 >= min_movement:
            break

    # Number of points to average for direction calculation
    num_points = 8
    end_index = min(i + num_points, len(x_positions))

    # Calculate average direction vector
    start_x = x_positions.iloc[start_index]
    start_y = y_positions.iloc[start_index]
    avg_dx = x_positions.iloc[start_index+1:end_index].mean() - start_x
    avg_dy = y_positions.iloc[start_index+1:end_index].mean() - start_y

    # Normalize direction vector and scale to arrow_length
    norm = (avg_dx**2 + avg_dy**2)**0.5
    if norm != 0:
        avg_dx = avg_dx / norm * arrow_length
        avg_dy = avg_dy / norm * arrow_length
    else:
        avg_dx = arrow_length
        avg_dy = 0
    plt.quiver(start_x, start_y, avg_dx, avg_dy, angles='xy', scale_units='xy', scale=0.3, color='red', label='Start Direction', zorder=5)
    # Overlay Savitzky-Golay smoothed trajectory
    plt.plot(smoothed_x, smoothed_y, label='Smoothed Trajectory', color='blue', linewidth=1, alpha=1, zorder=4)
    plt.xlabel("X Position (m)")
    plt.ylabel("Y Position (m)")
    plt.title("Marker 10 Position Plot")
    plt.legend()
    plt.grid(which='both', linestyle='--', linewidth=0.5)
    plt.axis('equal')
    # Plot positions with moving average filter
    #moving_avg_x_positions = x_positions.rolling(window=window_size).mean()
    #moving_avg_y_positions = y_positions.rolling(window=window_size).mean()
    plt.plot(label='Moving Average Position', color='orange')
    plt.legend()
    
    # Save plot
    plt.tight_layout()
    plt.savefig(output_plot_path.replace('.png', '_trajectory.png'))
    plt.close()
    
    print(f"Plots saved to {os.path.dirname(output_plot_path)}")

if __name__ == "__main__":
    input_folder = os.path.join(os.path.dirname(__file__), 'output_files_repaired')
    output_folder = os.path.join(os.path.dirname(__file__), 'output_plots')
    os.makedirs(output_folder, exist_ok=True)
    
    for file_name in os.listdir(input_folder):
        if file_name.endswith('.csv'):
            csv_file = os.path.join(input_folder, file_name)
            plot_output = os.path.join(output_folder, file_name.replace('.csv', '.png'))
            plot_marker_data(csv_file, plot_output)
