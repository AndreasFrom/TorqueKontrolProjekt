import cv2
import numpy as np
import os
import csv
from tqdm import tqdm
import sys
from collections import deque

# Constants
CIRCLE_MARKER = 11  # Marks the center of the circle
CAR_MARKER = 10     # Marks the car position
MAX_DISTANCE_THRESHOLD = 200  # Maximum allowed distance between consecutive points in pixels
MOVING_AVG_WINDOW = 10  # Number of frames to average over

def initialize_video(video_path, output_path_video, frame_width, frame_height, fps):
    try:
        cap = cv2.VideoCapture(video_path)
        if not cap.isOpened():
            raise IOError(f"Could not open video file: {video_path}")
        
        # Try to read the first frame to verify the video can be processed
        ret, _ = cap.read()
        if not ret:
            raise ValueError("Video file appears to be empty or corrupted")
        cap.set(cv2.CAP_PROP_POS_FRAMES, 0)  # Reset to first frame
        
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        out = cv2.VideoWriter(output_path_video, fourcc, fps, (frame_width, frame_height))
        if not out.isOpened():
            raise IOError(f"Could not create output video file: {output_path_video}")
            
        return cap, out
    except Exception as e:
        print(f"Error during video initialization: {str(e)}")
        sys.exit(1)

def detect_markers(detector, frame, valid_ids, tracked_markers):
    try:
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        corners, ids, _ = detector.detectMarkers(gray)
        detected_ids = set(ids.flatten()) if ids is not None else set()

        if ids is not None:
            for i, marker_id in enumerate(ids.flatten()):
                if marker_id in valid_ids:
                    # Calculate current marker center position
                    current_center = np.mean(corners[i][0], axis=0)
                    
                    if marker_id not in tracked_markers:
                        # Initialize tracking data structure
                        tracked_markers[marker_id] = {
                            'corners': corners[i],
                            'last_known_corners': corners[i],
                            'position_history': deque(maxlen=MOVING_AVG_WINDOW),
                            'last_valid_position': current_center,
                            'smoothed_position': None
                        }
                        # Initialize history with current position
                        tracked_markers[marker_id]['position_history'].append(current_center)
                    else:
                        # Check distance from last valid position
                        last_pos = tracked_markers[marker_id]['last_valid_position']
                        distance = np.linalg.norm(current_center - last_pos) if last_pos is not None else 0
                        
                        if last_pos is None or distance <= MAX_DISTANCE_THRESHOLD:
                            # Accept this detection
                            tracked_markers[marker_id]['corners'] = corners[i]
                            tracked_markers[marker_id]['last_known_corners'] = corners[i]
                            tracked_markers[marker_id]['position_history'].append(current_center)
                            tracked_markers[marker_id]['last_valid_position'] = current_center
                            
                            # Calculate moving average if we have enough samples
                            if len(tracked_markers[marker_id]['position_history']) >= MOVING_AVG_WINDOW:
                                avg_position = np.mean(tracked_markers[marker_id]['position_history'], axis=0)
                                tracked_markers[marker_id]['smoothed_position'] = avg_position
                        else:
                            # Reject this detection as too far away
                            print(f"Marker {marker_id} moved too far ({distance:.2f} pixels), using last valid position")
                            corners = list(corners)  # Convert to a mutable list
                            corners[i] = tracked_markers[marker_id]['last_known_corners']
                            corners = tuple(corners)  # Convert back to a tuple

        # Handle markers that weren't detected in this frame
        for marker_id in list(tracked_markers.keys()):
            if marker_id not in detected_ids:
                if tracked_markers[marker_id]['smoothed_position'] is not None:
                    # Use smoothed position if available
                    pass  # We'll keep using the last known corners
                else:
                    # Fall back to last known corners
                    tracked_markers[marker_id]['corners'] = tracked_markers[marker_id]['last_known_corners']

        return corners, ids, detected_ids
    except Exception as e:
        print(f"Error during marker detection: {str(e)}")
        return None, None, set()

def process_corners(tracked_markers, corner_ids):
    try:
        selected_corners = []
        selected_ids = []
        other_corners = []
        other_ids = []

        for marker_id, marker_data in tracked_markers.items():
            if marker_id in corner_ids:
                selected_corners.append(marker_data['corners'][0])
                selected_ids.append(marker_id)
            else:
                other_corners.append(marker_data['corners'][0])
                other_ids.append(marker_id)

        return selected_corners, selected_ids, other_corners, other_ids
    except Exception as e:
        print(f"Error during corner processing: {str(e)}")
        return [], [], [], []

def warp_frame(frame, selected_corners, frame_width, frame_height):
    try:
        if len(selected_corners) != 4:
            raise ValueError("Exactly 4 corners are required for warping")
            
        points = np.array([corner for corner in selected_corners], dtype=np.float32)
        sorted_points = points[np.argsort(points[:, 0, 0])]
        left_points = sorted_points[:2][np.argsort(sorted_points[:2][:, 0, 1])]
        right_points = sorted_points[2:][np.argsort(sorted_points[2:][:, 0, 1])]
        top_left, bottom_left = left_points
        top_right, bottom_right = right_points
        ordered_points = np.array([top_left[0], top_right[0], bottom_right[0], bottom_left[0]], dtype=np.float32)
        dst_points = np.array([[0, 0], [frame_width-1, 0], [frame_width-1, frame_height-1], [0, frame_height-1]], dtype=np.float32)
        matrix = cv2.getPerspectiveTransform(ordered_points, dst_points)
        warped_image = cv2.warpPerspective(frame, matrix, (frame_width, frame_height))
        return warped_image, matrix
    except Exception as e:
        print(f"Error during frame warping: {str(e)}")
        return None, None

def calculate_marker_locations(other_ids, other_corners, matrix, frame_width, frame_height, real_world_distance, radius_meters, warped_image):
    try:
        marker_locations = {}
        circle_center = None

        for i in range(len(other_ids)):
            marker_point = other_corners[i][0].reshape(1, 1, -1)
            transformed_center = cv2.perspectiveTransform(marker_point, matrix)[0][0]
            
            # Calculate real-world coordinates
            x_meters = (transformed_center[0] / frame_width) * real_world_distance
            y_meters = (transformed_center[1] / frame_height) * real_world_distance
            marker_locations[other_ids[i]] = (x_meters, y_meters)

            text = f"ID {other_ids[i]}: ({x_meters:.2f}m, {y_meters:.2f}m)"
            cv2.putText(warped_image, text, (int(transformed_center[0]), int(transformed_center[1])), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2, cv2.LINE_AA)

            if other_ids[i] == CIRCLE_MARKER:
                radius_pixels = int((radius_meters / real_world_distance) * frame_height)
                cv2.circle(warped_image, (int(transformed_center[0]), int(transformed_center[1])), radius_pixels, (0, 0, 255), 2)
                circle_center = (x_meters, y_meters)

        return marker_locations, circle_center
    except Exception as e:
        print(f"Error during marker location calculation: {str(e)}")
        return {}, None

def save_to_csv(data, output_csv):
    try:
        os.makedirs(os.path.dirname(output_csv), exist_ok=True)
        header = ["fps", "Frame", "Marker ID", "X Position (m)", "Y Position (m)", "Distance from Circle (m)", "Distance from Previous Point (m)", "Speed (m/s)"]
        with open(output_csv, mode="w", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(header)
            writer.writerows(data)
        print(f"Success: CSV file saved at {output_csv}")
    except Exception as e:
        print(f"Error saving CSV file: {str(e)}")

def main():
    try:
        # Create output directory if it doesn't exist
        output_dir = os.path.join(os.path.dirname(__file__), 'output_files')
        os.makedirs(output_dir, exist_ok=True)
        
        video_path = 'input_files/IMG_4387.mov'
        output_path = os.path.join(output_dir, 'output_warped_video.mp4')
        output_csv = os.path.join(output_dir, 'marker_positions.csv')

        # Check if input file exists
        if not os.path.exists(video_path):
            raise FileNotFoundError(f"Input video file not found: {video_path}")

        frame_width, frame_height = 400, 400
        real_world_distance = 1.975  # Distance between corner markers in meters
        radius_meters = 0.5      # Radius of the circle in meters
        corner_ids = [6, 7, 8, 9]
        valid_ids = set(range(6, 12))  # Valid marker IDs (6-11)

        cap = cv2.VideoCapture(video_path)
        if not cap.isOpened():
            raise IOError(f"Could not open video file: {video_path}")
            
        fps = int(cap.get(cv2.CAP_PROP_FPS))
        total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        cap.release()

        cap, out = initialize_video(video_path, output_path, frame_width, frame_height, fps)

        aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
        parameters = cv2.aruco.DetectorParameters()
        detector = cv2.aruco.ArucoDetector(aruco_dict, parameters)
        tracked_markers = {}

        frame_index = 0
        csv_data = []  
        last_circle_center = None  
        missing_corners_count = 0
        total_processed_frames = 0

        previous_x = None
        previous_y = None

        with tqdm(total=total_frames, desc="Processing Video", unit="frame") as pbar:
            while True:
                ret, frame = cap.read()
                if not ret:
                    break

                corners, ids, detected_ids = detect_markers(detector, frame, valid_ids, tracked_markers)
                selected_corners, selected_ids, other_corners, other_ids = process_corners(tracked_markers, corner_ids)

                if len(selected_corners) == 4:
                    warped_image, matrix = warp_frame(frame, selected_corners, frame_width, frame_height)
                    if warped_image is not None:
                        marker_locations, circle_center = calculate_marker_locations(
                            other_ids, other_corners, matrix, frame_width, frame_height, real_world_distance, radius_meters, warped_image
                        )

                        if circle_center is not None:
                            last_circle_center = circle_center

                        out.write(warped_image)
                        total_processed_frames += 1

                        if CAR_MARKER in marker_locations and last_circle_center is not None:
                            x, y = marker_locations[CAR_MARKER]
                            distance_from_circle = np.sqrt((x - last_circle_center[0])**2 + (y - last_circle_center[1])**2)
                            dsitance_from_prevoius_point = np.sqrt((x - previous_x)**2 + (y - previous_y)**2) if previous_x is not None and previous_y is not None else 0
                            speed = dsitance_from_prevoius_point / (1 / fps)
                            csv_data.append((fps, frame_index, CAR_MARKER, x, y, distance_from_circle, dsitance_from_prevoius_point,speed))

                            previous_x = x 
                            previous_y = y 
                else:
                    missing_corners_count += 1

                frame_index += 1
                pbar.update(1)

        cap.release()
        out.release()

        if total_processed_frames == 0:
            raise RuntimeError("No frames were successfully processed. Check if markers are visible and configuration is correct.")
            
        if missing_corners_count > 0:
            print(f"Warning: Could not detect all 4 corner markers in {missing_corners_count} frames")

        save_to_csv(csv_data, output_csv)

        print(f"\nSuccess: Warped video saved to {output_path}")
        print(f"Processed {total_processed_frames}/{total_frames} frames successfully")
        print(f"Processing complete. Results saved in {output_dir}")

    except Exception as e:
        print(f"\nError: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main()