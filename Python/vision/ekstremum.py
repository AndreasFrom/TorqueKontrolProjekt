import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import find_peaks
from scipy.signal import savgol_filter
from typing import List, Tuple, Dict
import os
from collections import defaultdict

def remove_iqr_outliers(values: np.ndarray, times: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """Remove IQR outliers while keeping time-value pairs aligned."""
    q1 = np.percentile(values, 25)
    q3 = np.percentile(values, 75)
    iqr = q3 - q1
    lower = q1 - 1.5 * iqr
    upper = q3 + 1.5 * iqr

    mask = (values >= lower) & (values <= upper)
    return values[mask], times[mask]



def load_data(csv_path: str) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    required_columns = ["fps", "Frame", "X Position (m)", "Y Position (m)"]
    for col in required_columns:
        if col not in df.columns:
            raise ValueError(f"Missing column: {col}")
    return df


def get_peaks_and_lows(x_positions: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    peaks, _ = find_peaks(x_positions, distance=5, prominence=0.3, width=2)
    lows, _ = find_peaks(-x_positions, distance=3, prominence=0.1, width=1)
    return peaks, lows


def euclidean_distance(x1, y1, x2, y2) -> float:
    return np.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)


def plot_distances_2d_time(
    seconds: np.ndarray,
    x_positions: np.ndarray,
    y_positions: np.ndarray,
    peaks: np.ndarray,
    lows: np.ndarray,
    title: str = "Circle Diameter Measurements Over Time",
    save_path: str = None
):
    """Plot diameter measurements over time for a single file"""
    if len(peaks) < 1 or len(lows) < 1:
        print("Need both peaks and lows to calculate diameters.")
        return

    diameters = []
    measurement_times = []
    pairs = []
    is_peak_first = peaks[0] < lows[0]

    for i in range(max(len(peaks), len(lows)) - 1):
        if is_peak_first:
            if i < len(peaks) and i < len(lows):
                pairs.append((peaks[i], lows[i]))
            if i < len(lows) and i + 1 < len(peaks):
                pairs.append((lows[i], peaks[i + 1]))
        else:
            if i < len(lows) and i < len(peaks):
                pairs.append((lows[i], peaks[i]))
            if i < len(peaks) and i + 1 < len(lows):
                pairs.append((peaks[i], lows[i + 1]))

    for p1, p2 in pairs:
        distance = euclidean_distance(x_positions[p1], y_positions[p1],
                                      x_positions[p2], y_positions[p2])
        diameters.append(distance)
        measurement_times.append((seconds[p1] + seconds[p2]) / 2)

    if not diameters:
        print("No valid diameter measurements found.")
        return

    times_array = np.array(measurement_times)
    diameters_array = np.array(diameters)

    # Filter using IQR
    diameters_array, times_array = remove_iqr_outliers(diameters_array, times_array)

    sort_idx = np.argsort(times_array)
    times_sorted = times_array[sort_idx]
    diameters_sorted = diameters_array[sort_idx]


    plt.figure(figsize=(14, 7))
    plt.scatter(times_sorted, diameters_sorted, c='blue', alpha=0.6, label='Diameter Measurements')

    plt.xlabel("Time (s)")
    plt.ylabel("Diameter (m)")
    plt.title(f"{title}\n{len(diameters)} measurements")
    plt.grid(True, alpha=0.3)
    plt.legend()

    stats_text = (f"Mean: {np.mean(diameters):.3f} m\n"
                  f"Std Dev: {np.std(diameters):.3f} m\n"
                  f"Min: {np.min(diameters):.3f} m\n"
                  f"Max: {np.max(diameters):.3f} m")
    plt.gca().text(0.02, 0.98, stats_text,
                   transform=plt.gca().transAxes,
                   verticalalignment='top',
                   bbox=dict(facecolor='white', alpha=0.8))

    plt.tight_layout()
    if save_path:
        plt.savefig(save_path, dpi=300)
        plt.close()
    else:
        plt.show()


def analyze_motion(csv_file: str) -> Dict[str, np.ndarray]:
    """Analyze motion data and return relevant arrays"""
    df = pd.read_csv(csv_file)
    fps = df["fps"].iloc[0]
    frames = df["Frame"]
    x_positions = df["X Position (m)"].values
    y_positions = df["Y Position (m)"].values
    seconds = frames / fps

    # Apply Savitzky-Golay filter
    window_size = int(fps * 0.5)
    if window_size % 2 == 0:
        window_size += 1  # must be odd
    smoothed_x = savgol_filter(x_positions, window_length=window_size, polyorder=2)
    smoothed_y = savgol_filter(y_positions, window_length=window_size, polyorder=2)

    # Find peaks/lows in smoothed data
    x_peaks, _ = find_peaks(smoothed_x, distance=5, prominence=0.3)
    x_lows, _ = find_peaks(-smoothed_x, distance=3, prominence=0.1)

    return {
        'seconds': seconds,
        'smoothed_x': smoothed_x,
        'smoothed_y': smoothed_y,
        'x_peaks': x_peaks,
        'x_lows': x_lows,
        'filename': os.path.basename(csv_file)
    }


def plot_grouped_diameters(group_data: List[Dict[str, np.ndarray]], output_path: str):
    """Plot all diameter measurements for a group of files in one figure"""
    plt.figure(figsize=(14, 10))
    
    all_diameters = []
    
    for i, data in enumerate(group_data):
        seconds = data['seconds']
        x = data['smoothed_x']
        y = data['smoothed_y']
        peaks = data['x_peaks']
        lows = data['x_lows']
        filename = data['filename']
        
        if len(peaks) < 1 or len(lows) < 1:
            print(f"Skipping {filename} - not enough peaks/lows")
            continue

        diameters = []
        measurement_times = []
        pairs = []
        is_peak_first = peaks[0] < lows[0]

        for i in range(max(len(peaks), len(lows)) - 1):
            if is_peak_first:
                if i < len(peaks) and i < len(lows):
                    pairs.append((peaks[i], lows[i]))
                if i < len(lows) and i + 1 < len(peaks):
                    pairs.append((lows[i], peaks[i + 1]))
            else:
                if i < len(lows) and i < len(peaks):
                    pairs.append((lows[i], peaks[i]))
                if i < len(peaks) and i + 1 < len(lows):
                    pairs.append((peaks[i], lows[i + 1]))

        for p1, p2 in pairs:
            distance = euclidean_distance(x[p1], y[p1], x[p2], y[p2])
            diameters.append(distance)
            measurement_times.append((seconds[p1] + seconds[p2]) / 2)

        if not diameters:
            print(f"No valid diameter measurements found in {filename}")
            continue

        times_array = np.array(measurement_times)        
        diameters_array = np.array(diameters)

        for i in range(len(diameters_array)):
            # if data < 0.9 remove it
            # Remove diameter measurements less than 0.9
            mask = diameters_array >= 0.9
            diameters_array = diameters_array[mask]
            times_array = times_array[mask]

        diameters_array, times_array = remove_iqr_outliers(diameters_array, times_array)


        sort_idx = np.argsort(times_array)
        times_sorted = times_array[sort_idx]
        diameters_sorted = diameters_array[sort_idx]
        
        all_diameters.extend(diameters_sorted)
        
        # Plot individual file data
        plt.scatter(times_sorted, diameters_sorted, 
                   label=f"{filename} (n={len(diameters_sorted)})")
    
    if not all_diameters:
        print("No valid diameter measurements in this group")
        plt.close()
        return
    
    # Calculate and plot overall statistics
    mean_diameter = np.mean(all_diameters)
    std_diameter = np.std(all_diameters)
    
    plt.axhline(mean_diameter, color='r', linestyle='--', label=f'Group Mean: {mean_diameter:.3f}m')
    plt.axhline(mean_diameter + std_diameter, color='gray', linestyle=':', label=f'±1 Std Dev')
    plt.axhline(mean_diameter - std_diameter, color='gray', linestyle=':')
    
    plt.xlabel("Time (s)")
    plt.ylabel("Diameter (m)")
    plt.title(f"Diameter Measurements - {group_data[0]['filename'].split('_')[0]}_{group_data[0]['filename'].split('_')[1]}\n"
              f"Total measurements: {len(all_diameters)}")
    plt.grid(True, alpha=0.3)
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    
    stats_text = (f"Group Statistics:\n"
                 f"Mean: {mean_diameter:.3f} m\n"
                 f"Std Dev: {std_diameter:.3f} m\n"
                 f"Min: {np.min(all_diameters):.3f} m\n"
                 f"Max: {np.max(all_diameters):.3f} m")
    plt.gca().text(0.02, 0.98, stats_text,
                  transform=plt.gca().transAxes,
                  verticalalignment='top',
                  bbox=dict(facecolor='white', alpha=0.8))
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()


def main():
    input_folder = os.path.join(os.path.dirname(__file__), 'output_files_repaired')
    output_base_folder = os.path.join(os.path.dirname(__file__), 'output_plots')
    os.makedirs(output_base_folder, exist_ok=True)

    # Group files by their prefix (e.g., '05_speed', '075_ICOtorque')
    file_groups = defaultdict(list)
    
    for file_name in os.listdir(input_folder):
        if file_name.endswith('.csv'):
            parts = file_name.split('_')
            if len(parts) >= 2:
                prefix = f"{parts[0]}_{parts[1]}"
                file_groups[prefix].append(os.path.join(input_folder, file_name))

    # Process each group
    for prefix, file_list in file_groups.items():
        print(f"\nProcessing group: {prefix}")
        group_output_folder = os.path.join(output_base_folder, prefix)
        os.makedirs(group_output_folder, exist_ok=True)
        
        # Analyze all files in the group
        group_data = []
        for file_path in file_list:
            print(f"  Analyzing {os.path.basename(file_path)}...")
            try:
                data = analyze_motion(file_path)
                group_data.append(data)
                
                # Save individual plots
                plot_distances_2d_time(
                    data['seconds'], 
                    data['smoothed_x'], 
                    data['smoothed_y'], 
                    data['x_peaks'], 
                    data['x_lows'],
                    title=f"Diameter Measurements - {os.path.basename(file_path)}",
                    save_path=os.path.join(group_output_folder, f"{os.path.splitext(os.path.basename(file_path))[0]}_radii.png")
                )
            except Exception as e:
                print(f"Error processing {file_path}: {str(e)}")
                continue
        
        # Create combined plot for the group
        if group_data:
            combined_plot_path = os.path.join(group_output_folder, f"{prefix}_combined_diameters.png")
            plot_grouped_diameters(group_data, combined_plot_path)
            print(f"Saved combined plot to {combined_plot_path}")


if __name__ == "__main__":
    main()