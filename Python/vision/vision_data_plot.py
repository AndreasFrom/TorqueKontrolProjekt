import matplotlib.pyplot as plt
import pandas as pd
import os

def plot_marker_data(csv_path, output_plot_path):
    # Load data
    df = pd.read_csv(csv_path)
    
    # Extract columns
    fps = df["fps"]
    frames = df["Frame"]
    x_positions = df["X Position (m)"]
    y_positions = df["Y Position (m)"]
    speed = df["Speed (m/s)"]

    distances = df["Distance from Circle (m)"]
    seconds = frames*1/fps
    
    # Create plots
    fig_pos_time, axes = plt.subplots(2, 1, figsize=(10, 8))
    
    # Plot X and Y positions
    axes[0].plot(seconds, x_positions, label='X Position', color='b')
    axes[0].plot(seconds, y_positions, label='Y Position', color='g')
    axes[0].set_xlabel("s")
    axes[0].set_ylabel("Position (m)")
    axes[0].set_title("Marker 10 Position Over Time")
    axes[0].legend()
    axes[0].grid()
    
    # Plot Distance from Circle
    axes[1].plot(seconds, distances, label='Distance from Circle', color='r')
    axes[1].set_xlabel("s")
    axes[1].set_ylabel("Distance (m)")
    axes[1].set_title("Marker 10 Distance from Circle Over Time")
    axes[1].legend()
    axes[1].grid()
    
    # Save
    fig_pos_time.tight_layout()
    fig_pos_time.savefig(output_plot_path.replace('.png', '_position.png'))
    plt.close(fig_pos_time)
    
    # Create speed plot
    fig_speed, axes = plt.subplots(2, 1, figsize=(10, 4))
    
    # Plot speed
    axes[0].plot(seconds, speed, label='Speed', color='purple')
    axes[0].set_xlabel("s")
    axes[0].set_ylabel("Speed (m/s)")
    axes[0].set_title("Marker 10 Speed Over Time")
    axes[0].legend()
    axes[0].grid()
    
    # Plot moving average speed
    window_size = int(fps.iloc[0] * 0.5)  # Set window size to 1 second based on FPS
    moving_avg_speed = speed.rolling(window=window_size).mean()
    axes[1].plot(seconds, moving_avg_speed, label='Moving Average Speed', color='orange')
    axes[1].set_xlabel("s")
    axes[1].set_ylabel("Speed (m/s)")
    axes[1].set_title(f"Marker 10 Speed Moving Average (Window Size: {window_size})")
    axes[1].legend()
    axes[1].grid()
    
    # Save
    fig_speed.tight_layout()
    fig_speed.savefig(output_plot_path.replace('.png', '_speed.png'))
    plt.close(fig_pos_time)

    
    # Plot positions
    plt.figure(figsize=(6, 6))  # Set figure size to ensure a square plot
    plt.plot(x_positions, y_positions, label='Position', color='blue')
    
    # Add a vector arrow to indicate the starting direction and location using the first few points
    num_points = 5  # Number of points to calculate the average direction
    start_x = x_positions.iloc[0]
    start_y = y_positions.iloc[0]
    avg_dx = x_positions.iloc[1:num_points].mean() - start_x
    avg_dy = y_positions.iloc[1:num_points].mean() - start_y
    plt.quiver(start_x, start_y, avg_dx, avg_dy, angles='xy', scale_units='xy', scale=1, color='red', label='Start Direction')
    plt.xlabel("X Position (m)")
    plt.ylabel("Y Position (m)")
    plt.title("Marker 10 Position Plot")
    plt.legend()
    plt.grid()
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
