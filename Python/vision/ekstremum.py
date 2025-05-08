import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import find_peaks
from typing import List, Tuple
import os


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


def compute_distances(peaks: np.ndarray, lows: np.ndarray, x: np.ndarray, y: np.ndarray) -> List[Tuple[float, float]]:
    distances = []
    is_peak_first = peaks[0] < lows[0]
    for i in range(min(len(peaks), len(lows)) - 1):
        if is_peak_first:
            p, l, np_ = peaks[i], lows[i], peaks[i + 1]
            d1 = euclidean_distance(x[p], y[p], x[l], y[l])
            d2 = euclidean_distance(x[l], y[l], x[np_], y[np_])
        else:
            l, p, nl = lows[i], peaks[i], lows[i + 1]
            d1 = euclidean_distance(x[l], y[l], x[p], y[p])
            d2 = euclidean_distance(x[p], y[p], x[nl], y[nl])
        distances.append((d1, d2))
        is_peak_first = not is_peak_first
    return distances


def plot_peaks_lows(seconds, x, peaks, lows, title="X Position with Peaks and Lows", save_path: str = None):
    plt.figure(figsize=(10, 5))
    plt.plot(seconds, x, label="X Position (m)", color="blue")
    plt.plot(seconds[peaks], x[peaks], "rx", label="Peaks")
    plt.plot(seconds[lows], x[lows], "go", label="Lows")
    plt.xlabel("Time (s)")
    plt.ylabel("X Position (m)")
    plt.title(title)
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300)
        plt.close()
    else:
        plt.show()


def plot_distances(seconds, x, peaks, lows, distances: List[Tuple[float, float]], save_path: str = None):
    plt.figure(figsize=(10, 5))
    plt.plot(seconds, x, label="X Position (m)", color="blue")
    plt.plot(seconds[peaks], x[peaks], "rx", label="Peaks")
    plt.plot(seconds[lows], x[lows], "go", label="Lows")

    is_peak_first = peaks[0] < lows[0]
    for i, (d1, d2) in enumerate(distances):
        try:
            if is_peak_first:
                p, l, np_ = peaks[i], lows[i], peaks[i + 1]
                plt.plot([seconds[p], seconds[l]], [x[p], x[l]], "r--")
                plt.plot([seconds[l], seconds[np_]], [x[l], x[np_]], "g--")
            else:
                l, p, nl = lows[i], peaks[i], lows[i + 1]
                plt.plot([seconds[l], seconds[p]], [x[l], x[p]], "g--")
                plt.plot([seconds[p], seconds[nl]], [x[p], x[nl]], "r--")
        except IndexError:
            continue

    plt.title("X Position with Segment Distances")
    plt.xlabel("Time (s)")
    plt.ylabel("X Position (m)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=300)
        plt.close()
    else:
        plt.show()


def plot_distances_2d_time(
    seconds: np.ndarray,
    x_positions: np.ndarray,
    y_positions: np.ndarray,
    peaks: np.ndarray,
    lows: np.ndarray,
    title: str = "Circle Diameter Measurements Over Time",
    save_path: str = None
):
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


def analyze_motion(csv_file, output_dir=None):
    df = pd.read_csv(csv_file)
    fps = df["fps"].iloc[0]
    frames = df["Frame"]
    x_positions = df["X Position (m)"].values
    y_positions = df["Y Position (m)"].values
    seconds = frames / fps

    x_peaks, _ = find_peaks(x_positions, distance=5, prominence=0.3)
    x_lows, _ = find_peaks(-x_positions, distance=3, prominence=0.1)

    base_filename = os.path.splitext(os.path.basename(csv_file))[0]
    save_path = os.path.join(output_dir, f"{base_filename}_diameter_plot.png") if output_dir else None

    plot_distances_2d_time(seconds, x_positions, y_positions, x_peaks, x_lows, save_path=save_path)


if __name__ == "__main__":
    input_folder = os.path.join(os.path.dirname(__file__), 'output_files_repaired')
    output_folder = os.path.join(os.path.dirname(__file__), 'output_plots')
    os.makedirs(output_folder, exist_ok=True)

    for file_name in os.listdir(input_folder):
        if file_name.endswith('.csv'):
            csv_file = os.path.join(input_folder, file_name)
            print(f"Processing {csv_file}...")
            analyze_motion(csv_file, output_folder)
