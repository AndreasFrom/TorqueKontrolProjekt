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

def main():
    # Load data
    video_path = 'output_files/output_warped_video.mp4'
    csv_path = 'output_files/marker_positions.csv'
    
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise ValueError("Could not open video file")
    
    df = pd.read_csv(csv_path)
    df = df[df['Marker ID'] == CAR_MARKER]  # Filter for car data only
    
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
    ax2.set_ylim(0, 1)
    
    ax3 = plt.subplot2grid((2, 3), (1, 1))
    ax3.set_title('Y Position (m)')
    ax3.set_ylim(0, 1)
    
    ax4 = plt.subplot2grid((2, 3), (0, 2))
    ax4.set_title('Distance from Center (m)')
    ax4.set_ylim(0, 1)
    
    ax5 = plt.subplot2grid((2, 3), (1, 2))
    ax5.set_title('2D Position')
    ax5.set_xlim(0, 1)
    ax5.set_ylim(0, 1)
    
    # Initialize plots
    x_data, y_data, dist_data = [], [], []
    x_line, = ax2.plot([], [], 'b-')
    y_line, = ax3.plot([], [], 'g-')
    dist_line, = ax4.plot([], [], 'r-')
    pos_line, = ax5.plot([], [], 'b-', alpha=0.5)
    current_pos = ax5.plot([], [], 'bo')[0]
    
    # Add circle to position plot
    circle = plt.Circle((0.5, 0.5), 0.1, color='r', fill=False)
    ax5.add_patch(circle)
    
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
            
            # Get current frame data
            current_frame_data = df[df['Frame'] == frame_num]
            if not current_frame_data.empty:
                x = current_frame_data['X Position (m)'].values[0]
                y = current_frame_data['Y Position (m)'].values[0]
                dist = current_frame_data['Distance from Circle (m)'].values[0]
                
                x_data.append((frame_num, x))
                y_data.append((frame_num, y))
                dist_data.append((frame_num, dist))
                
                # Update plots
                x_line.set_data(*zip(*x_data))
                y_line.set_data(*zip(*y_data))
                dist_line.set_data(*zip(*dist_data))
                
                pos_line.set_data(*zip(*[(d[1][1], d[1][0]) for d in enumerate(x_data)]))
                current_pos.set_data([y], [x])
                
                # Adjust axes
                for ax, data in zip([ax2, ax3, ax4], [x_data, y_data, dist_data]):
                    if data:
                        ax.set_xlim(0, max(d[0] for d in data) + 1)
                        y_values = [d[1] for d in data]
                        ax.set_ylim(min(y_values) - 0.1, max(y_values) + 0.1)
        
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