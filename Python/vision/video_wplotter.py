import cv2
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import os

# Constants
CIRCLE_MARKER = 11
CAR_MARKER = 10  # Fixed typo from CAR_MARKER to CAR_MARKER
FRAME_WIDTH, FRAME_HEIGHT = 400, 400

def main():
    # Load data
    video_path = 'output_files/output_warped_video.mp4'
    csv_path = 'output_files/marker_positions.csv'
    
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise ValueError("Could not open video file")
    
    df = pd.read_csv(csv_path)
    df = df[df['Marker ID'] == CAR_MARKER]  # Filter for car data only
    
    # Get all data points upfront
    all_frames = df['Frame'].values
    all_x = df['X Position (m)'].values
    all_y = df['Y Position (m)'].values
    all_dist = df['Distance from Circle (m)'].values
    
    # Create figure with subplots
    plt.figure(figsize=(15, 8))
    plt.suptitle('Car Tracking Visualization', fontsize=16)
    
    # Video display
    ax1 = plt.subplot2grid((2, 3), (0, 0), rowspan=2)
    ax1.set_title('Video Feed')
    ax1.axis('off')
    
    # Data plots
    ax2 = plt.subplot2grid((2, 3), (0, 1))
    ax2.set_title('X Position (m)')
    
    ax3 = plt.subplot2grid((2, 3), (1, 1))
    ax3.set_title('Y Position (m)')
    
    ax4 = plt.subplot2grid((2, 3), (0, 2))
    ax4.set_title('Distance from Center (m)')
    
    ax5 = plt.subplot2grid((2, 3), (1, 2))
    ax5.set_title('2D Position')
    ax5.set_xlim(0, 1)
    ax5.set_ylim(0, 1)
    
    # Plot all data as faint lines
    ax2.plot(all_frames, all_x, 'b-', alpha=0.3)
    ax3.plot(all_frames, all_y, 'g-', alpha=0.3)
    ax4.plot(all_frames, all_dist, 'r-', alpha=0.3)
    ax5.plot(all_y, all_x, 'b-', alpha=0.1)
    
    # Initialize current position markers (these will be updated)
    x_point, = ax2.plot([], [], 'bo', markersize=8)
    y_point, = ax3.plot([], [], 'go', markersize=8)
    dist_point, = ax4.plot([], [], 'ro', markersize=8)
    pos_point, = ax5.plot([], [], 'ro', markersize=10)
    
    # Add circle to position plot
    circle = plt.Circle((0.5, 0.5), 0.1, color='r', fill=False)
    ax5.add_patch(circle)
    
    # Set axis limits based on all data
    ax2.set_xlim(0, max(all_frames) + 1)
    ax2.set_ylim(min(all_x) - 0.1, max(all_x) + 0.1)
    ax3.set_xlim(0, max(all_frames) + 1)
    ax3.set_ylim(min(all_y) - 0.1, max(all_y) + 0.1)
    ax4.set_xlim(0, max(all_frames) + 1)
    ax4.set_ylim(min(all_dist) - 0.1, max(all_dist) + 0.1)
    ax5.set_xlim(min(all_y) - 0.1, max(all_y) + 0.1)
    ax5.set_ylim(min(all_x) - 0.1, max(all_x) + 0.1)
    
    plt.tight_layout()
    
    # Frame slider
    ax_slider = plt.axes([0.2, 0.02, 0.6, 0.03])
    frame_slider = Slider(
        ax=ax_slider,
        label='Frame',
        valmin=0,
        valmax=int(cap.get(cv2.CAP_PROP_FRAME_COUNT)) - 1,
        valinit=0,
        valstep=1
    )
    
    def update(frame_num):
        cap.set(cv2.CAP_PROP_POS_FRAMES, frame_num)
        ret, frame = cap.read()
        if ret:
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            ax1.imshow(frame)
            
            # Find the index for the current frame
            idx = np.where(all_frames == frame_num)[0]
            if idx.size > 0:
                idx = idx[0]
                x = all_x[idx]
                y = all_y[idx]
                dist = all_dist[idx]
                current_frame = all_frames[idx]
                
                # Update position markers
                x_point.set_data([current_frame], [x])
                y_point.set_data([current_frame], [y])
                dist_point.set_data([current_frame], [dist])
                pos_point.set_data([y], [x])
        
        plt.draw()
    
    frame_slider.on_changed(update)
    
    # Initial update
    update(0)
    plt.show()
    
    cap.release()

if __name__ == "__main__":
    # Create output directory if it doesn't exist
    os.makedirs('output_files', exist_ok=True)
    main()