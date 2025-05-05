import matplotlib.pyplot as plt
import pandas as pd
import os

def plot_marker_data(csv_path, output_plot_path):
    # Load data
    df = pd.read_csv(csv_path)
    
    # Extract columns
    frames = df["Frame"]
    x_positions = df["X Position (m)"]
    y_positions = df["Y Position (m)"]
    distances = df["Distance from Circle (m)"]
    
    # Create plots
    fig, axes = plt.subplots(2, 1, figsize=(10, 8))
    
    # Plot X and Y positions
    axes[0].plot(frames, x_positions, label='X Position', color='b')
    axes[0].plot(frames, y_positions, label='Y Position', color='g')
    axes[0].set_xlabel("Frame")
    axes[0].set_ylabel("Position (m)")
    axes[0].set_title("Marker 11 Position Over Time")
    axes[0].legend()
    axes[0].grid()
    
    # Plot Distance from Circle
    axes[1].plot(frames, distances, label='Distance from Circle', color='r')
    axes[1].set_xlabel("Frame")
    axes[1].set_ylabel("Distance (m)")
    axes[1].set_title("Marker 11 Distance from Circle Over Time")
    axes[1].legend()
    axes[1].grid()
    
    # Adjust layout
    plt.tight_layout()
    
    # Save plot
    plt.savefig(output_plot_path)
    plt.show()
    print(f"Plot saved to {output_plot_path}")

if __name__ == "__main__":
    csv_file = os.path.join(os.path.dirname(__file__), 'output_files/marker_positions.csv')
    plot_output = os.path.join(os.path.dirname(__file__), 'output_files/marker_plot.png')
    
    plot_marker_data(csv_file, plot_output)