import matplotlib.pyplot as plt
import pandas as pd
import os
from mpl_toolkits.mplot3d import Axes3D

def plot_marker_data(csv_path, output_plot_path):
    try:
        print(f"Processing file: {csv_path}")
        df = pd.read_csv(csv_path)

        print(f"Columns found: {df.columns.tolist()}")

        # Check required columns
        required_columns = ["fps", "Frame", "X Position (m)", "Y Position (m)"]
        for col in required_columns:
            if col not in df.columns:
                raise ValueError(f"Missing column: {col}")

        # Safely extract FPS
        fps_value = df["fps"].iloc[0]
        if pd.isna(fps_value):
            raise ValueError("Invalid 'fps' value (NaN)")

        # Extract values
        fps = float(fps_value)
        frames = df["Frame"]
        x_positions = df["X Position (m)"]
        y_positions = df["Y Position (m)"]
        seconds = frames * (1 / fps)

        # Plot
        fig = plt.figure(figsize=(10, 8))
        ax = fig.add_subplot(111, projection='3d')

        ax.plot(x_positions, seconds, y_positions, label='Trajectory', color='blue', linewidth=1.5)

        ax.set_xlabel('X Position (m)')
        ax.set_ylabel('Time (s)')
        ax.set_zlabel('Y Position (m)')
        ax.set_title('3D Trajectory (X, Time, Y)')
        ax.legend()
        ax.grid(True)

        plt.tight_layout()
        plt.savefig(output_plot_path)
        plt.close()

        print(f"Saved plot to: {output_plot_path}")

    except Exception as e:
        print(f"Error processing {csv_path}: {e}")

if __name__ == "__main__":
    base_dir = os.path.dirname(__file__)
    input_folder = os.path.join(base_dir, 'output_files_repaired')
    output_folder = os.path.join(base_dir, 'output_plots')

    print("Script is running.")
    print(f"Looking in folder: {input_folder}")
    
    if not os.path.exists(input_folder):
        print(f"Input folder does not exist: {input_folder}")
    else:
        os.makedirs(output_folder, exist_ok=True)

        files = os.listdir(input_folder)
        print(f"Found {len(files)} files.")

        for file_name in files:
            print(f"Found file: {file_name}")
            if file_name.endswith('.csv'):
                csv_file = os.path.join(input_folder, file_name)
                plot_output = os.path.join(output_folder, file_name.replace('.csv', '_3d.png'))
                plot_marker_data(csv_file, plot_output)
