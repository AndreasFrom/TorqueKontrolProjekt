import cv2
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import os

# Constants
CIRCLE_MARKER = 11
CAR_MARKER = 10
FRAME_WIDTH, FRAME_HEIGHT = 400, 400

def rotate(x, y):
    return y, -x

def main():
    # Load data
    video_path = '.\vision\output_files\IMG_4416_warped_video.mp4'
    csv_path = '.\vision\output_files\IMG_4416_marker_positions.csv'

    
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise ValueError("Could not open video file")
    
    df = pd.read_csv(csv_path)
    df = df[df['Marker ID'] == CAR_MARKER]  # Filter for car data only
    
    # Get all data points
    all_frames = df['Frame'].values
    all_x = df['X Position (m)'].values
    all_y = df['Y Position (m)'].values
    all_dist = df['Distance from Circle (m)'].values
    
    # Rotate all positions for the 2D plot
    rotated_x, rotated_y = rotate(all_y, all_x)
    
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
    ax2.plot(all_frames, all_x, 'b-', alpha=0.3)
    
    ax3 = plt.subplot2grid((2, 3), (1, 1))
    ax3.set_title('Y Position (m)')
    ax3.plot(all_frames, all_y, 'g-', alpha=0.3)
    
    ax4 = plt.subplot2grid((2, 3), (0, 2))
    ax4.set_title('Distance from Center (m)')
    ax4.plot(all_frames, all_dist, 'r-', alpha=0.3)
    
    # 2D Position plot (rotated)
    ax5 = plt.subplot2grid((2, 3), (1, 2))
    ax5.set_title('2D Position (Rotated)')
    ax5.plot(rotated_x, rotated_y, 'b-', alpha=0.1)
    
    # Add rotated circle (original center at 0.5,0.5)
    circle_x, circle_y = rotate(0.5, 0.5)
    circle = plt.Circle((circle_x, circle_y), 0.1, color='r', fill=False)
    ax5.add_patch(circle)
    
    # Current position markers
    x_point, = ax2.plot([], [], 'bo', markersize=8)
    y_point, = ax3.plot([], [], 'go', markersize=8)
    dist_point, = ax4.plot([], [], 'ro', markersize=8)
    pos_point, = ax5.plot([], [], 'ro', markersize=10)
    
    # Set axis limits
    ax2.set_xlim(0, max(all_frames) + 1)
    ax2.set_ylim(min(all_x) - 0.1, max(all_x) + 0.1)
    ax3.set_xlim(0, max(all_frames) + 1)
    ax3.set_ylim(min(all_y) - 0.1, max(all_y) + 0.1)
    ax4.set_xlim(0, max(all_frames) + 1)
    ax4.set_ylim(min(all_dist) - 0.1, max(all_dist) + 0.1)
    ax5.set_xlim(min(rotated_x) - 0.1, max(rotated_x) + 0.1)
    ax5.set_ylim(min(rotated_y) - 0.1, max(rotated_y) + 0.1)
    ax5.set_aspect('equal')
    
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
            
            # Find current frame data
            idx = np.where(all_frames == frame_num)[0]
            if idx.size > 0:
                idx = idx[0]
                x = all_x[idx]
                y = all_y[idx]
                dist = all_dist[idx]
                current_frame = all_frames[idx]
                
                # Update markers
                x_point.set_data([current_frame], [x])
                y_point.set_data([current_frame], [y])
                dist_point.set_data([current_frame], [dist])
                
                # Update rotated position
                rot_x, rot_y = rotate(y, x)
                pos_point.set_data([rot_x], [rot_y])
        
        plt.draw()
    
    frame_slider.on_changed(update)
    update(0)
    plt.show()
    cap.release()

if __name__ == "__main__":
    os.makedirs('output_files', exist_ok=True)
    main()