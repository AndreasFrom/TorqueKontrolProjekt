import matplotlib.pyplot as plt
import pandas as pd
import os
from mpl_toolkits.mplot3d import Axes3D 

def plot_marker_data(csv_path: str, output_plot_path: str) -> None:
    """
    Loads motion data from a CSV file and creates a 3D plot (X, Time, Y) trajectory.
    Saves the plot as a PNG file to the specified output path.
    """
    try:
        print(f"Processing file: {csv_path}")
        df = pd.read_csv(csv_path)

        # Check required columns
        required_columns = ["fps", "Frame", "X Position (m)", "Y Position (m)"]
        for col in required_columns:
            if col not in df.columns:
                raise ValueError(f"Missing column: {col}")

        # Extract and validate FPS
        fps_value = df["fps"].iloc[0]
        if pd.isna(fps_value):
            raise ValueError("Invalid 'fps' value (NaN)")
        fps = float(fps_value)

        # Extract data
        frames = df["Frame"].values
        x_positions = df["X Position (m)"].values
        y_positions = df["Y Position (m)"].values
        seconds = frames / fps

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
        plt.savefig(output_plot_path, dpi=300)
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
        csv_files = [f for f in files if f.endswith('.csv')]

        print(f"Found {len(csv_files)} CSV files.")

        for file_name in csv_files:
            csv_file = os.path.join(input_folder, file_name)
            plot_output = os.path.join(output_folder, file_name.replace('.csv', '_3d.png'))
            plot_marker_data(csv_file, plot_output)
