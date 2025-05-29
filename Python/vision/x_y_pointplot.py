import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import os

def plot_xy_with_stats(csv_path, output_plot_path):
    # Load data
    df = pd.read_csv(csv_path)

    # Extract X and Y
    x = df["X Position (m)"]
    y = df["Y Position (m)"]

    # Compute statistics
    stats = {
        "X Mean": x.mean(),
        "X Std": x.std(),
        "X Min": x.min(),
        "X Max": x.max(),
        "Y Mean": y.mean(),
        "Y Std": y.std(),
        "Y Min": y.min(),
        "Y Max": y.max(),
    }

    # Plot
    plt.figure(figsize=(8, 8))
    plt.plot(x, y, marker='o', linestyle='-', color='blue', label='XY Path')
    plt.xlabel("X Position (m)")
    plt.ylabel("Y Position (m)")
    plt.title("X-Y Position Plot")
    plt.axis('equal')
    plt.grid(True)
    plt.legend()

    # Add stats as a text box
    stats_text = '\n'.join([f"{k}: {v:.4f}" for k, v in stats.items()])
    plt.gca().text(
        0.02, 0.98, stats_text,
        transform=plt.gca().transAxes,
        fontsize=10,
        verticalalignment='top',
        bbox=dict(facecolor='white', alpha=0.8, boxstyle='round,pad=0.5')
    )

    # Save as JPEG
    plt.tight_layout()
    plt.savefig(output_plot_path, format='jpeg')
    plt.close()
    print(f"Saved plot with stats to: {output_plot_path}")

if __name__ == "__main__":
    input_folder = "output_files_repaired"
    output_folder = "output_plots"
    os.makedirs(output_folder, exist_ok=True)

    for file_name in os.listdir(input_folder):
        if file_name.endswith(".csv"):
            input_csv = os.path.join(input_folder, file_name)
            output_jpeg = os.path.join(output_folder, f"{os.path.splitext(file_name)[0]}_xy_plot.jpeg")
            plot_xy_with_stats(input_csv, output_jpeg)
