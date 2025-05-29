import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt
from scipy.signal import medfilt


def detect_spikes_and_repair(csv_path, output_nan_path, output_interpolated_path,
                              z_threshold=1.5, plot_results=False):
    # Load data
    df = pd.read_csv(csv_path)

    # Flip Y-axis
    df["Y Position (m)"] = df["Y Position (m)"].max() - df["Y Position (m)"]

    # Columns to check for spikes
    columns_to_check = ["X Position (m)", "Y Position (m)", "Speed (m/s)", "Distance from Circle (m)"]

    # Create a copy for NaN insertion
    df_nan = df.copy()

    for col in columns_to_check:
        # Median filter (optional, for baseline smoothing)
        baseline = pd.Series(medfilt(df[col], kernel_size=5))
        rolling_median = baseline.rolling(window=10, min_periods=1).median()

        # Detect large deviations
        diffs = np.abs(df[col] - rolling_median)
        threshold = diffs.median() * 3  # Tune multiplier if needed

        spike_indices = diffs > threshold
        df_nan.loc[spike_indices, col] = np.nan

    # Save the version with NaNs where spikes were detected
    df_nan.to_csv(output_nan_path, index=False)

    # Create interpolated version
    df_interpolated = df_nan.interpolate(method='linear', limit_direction='both')
    df_interpolated.to_csv(output_interpolated_path, index=False)

    print(f"Saved NaN version to: {output_nan_path}")
    print(f"Saved interpolated version to: {output_interpolated_path}")

    # Plotting before/after repair
    if plot_results:
        for col in columns_to_check:
            plt.figure(figsize=(10, 4))
            plt.plot(df.index, df[col], label="Original", alpha=0.5)
            plt.plot(df_interpolated.index, df_interpolated[col], label="Repaired", alpha=0.7)
            plt.title(f"{col} - Before and After Spike Repair")
            plt.xlabel("Index")
            plt.ylabel(col)
            plt.legend()
            plt.tight_layout()
            plt.show()


if __name__ == "__main__":
    input_folder = os.path.join(os.path.dirname(__file__), 'output_files')
    output_folder_nan = os.path.join(os.path.dirname(__file__), 'output_files_nan')
    output_folder_interp = os.path.join(os.path.dirname(__file__), 'output_files_repaired')

    os.makedirs(output_folder_nan, exist_ok=True)
    os.makedirs(output_folder_interp, exist_ok=True)

    for file_name in os.listdir(input_folder):
        if file_name.endswith('.csv'):
            input_path = os.path.join(input_folder, file_name)
            output_nan_path = os.path.join(output_folder_nan, file_name)
            output_interpolated_path = os.path.join(output_folder_interp, file_name)
            
            detect_spikes_and_repair(input_path, output_nan_path, output_interpolated_path, plot_results=True)
