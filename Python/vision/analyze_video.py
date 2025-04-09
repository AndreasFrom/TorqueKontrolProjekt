import cv2
import numpy as np
import os
import csv
from tqdm import tqdm
import sys
from collections import deque
import argparse

num_iterations = 0
debug = 0  # Global variable to control debug mode
# Constants
CIRCLE_MARKER = 11  # Marks the center of the circle
CAR_MARKER = 10     # Marks the car position
MAX_DISTANCE_THRESHOLD = 500  # Maximum allowed distance between consecutive points in pixels
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
        global num_iterations

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Use CLAHE instead of convertScaleAbs for better contrast
        clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(4,4))
        gray = clahe.apply(gray)

        # Add morphology to view though cable
        kernel = np.ones((5, 5), np.uint8)
        gray = cv2.morphologyEx(gray, cv2.MORPH_CLOSE, kernel)
        gray = cv2.morphologyEx(gray, cv2.MORPH_OPEN, kernel)
        gray = cv2.morphologyEx(gray, cv2.MORPH_DILATE, kernel)
        gray = cv2.morphologyEx(gray, cv2.MORPH_ERODE, kernel)

        # Save debug frame
        if debug >= 3:
            if num_iterations % 10 == 0:
                script_dir = os.path.dirname(os.path.abspath(__file__))
                debug_path = os.path.join(script_dir, f"debug_frame_{num_iterations}.jpg")
                cv2.imwrite(debug_path, gray)
                print(f"Debug frame saved to {debug_path}")

            num_iterations += 1

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
                            if debug >= 1:
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

        return True
    except Exception as e:
        print(f"Error during marker detection: {str(e)}")
        return False

def process_markers(tracked_markers, corner_ids):
    selected_corners = []
    selected_ids = []
    object_corners = []
    object_ids = []

    for marker_id, marker_data in tracked_markers.items():
        if marker_id in corner_ids:
            selected_corners.append(marker_data['corners'][0])
            selected_ids.append(marker_id)
        elif marker_id == CAR_MARKER:
            # For the car marker, calculate the mean of all corners as its position
            car_corners = np.array(marker_data['corners'])
            car_center = np.mean(car_corners, axis=0)  # Mean of all corners
            object_corners.append(car_center)  # This is the car's center
            object_ids.append(marker_id)
        else:
            # For other markers, just take the top-left corner
            object_corners.append(marker_data['corners'][0])  # Top-left corner
            object_ids.append(marker_id)

    return selected_corners, selected_ids, object_corners, object_ids

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

def calculate_marker_locations(object_ids, object_corners, matrix, frame_width, frame_height, real_world_distance, radius_meters, warped_image):
    try:
        marker_locations = {}  # Dictionary to hold the locations of the markers
        circle_center = None   # Placeholder for circle center (if needed for your application)

        for i in range(len(object_ids)):
            # Convert corners into a numpy array for easier handling
            corners = np.array(object_corners[i])

            # Handle car marker separately
            if object_ids[i] == CAR_MARKER:
                # For the car marker, take the average of its corners to get the center
                car_corners = corners  # Access the correct car corners
                
                # If corners are in shape (4, 1, 2), squeeze to (4, 2)
                if car_corners.ndim > 2:
                    car_corners = car_corners.squeeze()
                
                # Take the average to find the center of the car marker
                marker_center = np.mean(car_corners, axis=0)  # Calculate center of the marker
                
                # Reshape the center to (1, 1, 2) for perspective transformation
                marker_point = marker_center.reshape(1, 1, 2)

            else:
                marker_point = marker_center.reshape(1, 1, 2)

            # Perform the perspective transformation to get the real-world location
            transformed_center = cv2.perspectiveTransform(marker_point, matrix)[0][0]

            # Calculate real-world coordinates
            x_meters = (transformed_center[0] / frame_width) * real_world_distance
            y_meters = (transformed_center[1] / frame_height) * real_world_distance
            marker_locations[object_ids[i]] = (x_meters, y_meters)

            # Display the marker location on the warped image
            text = f"ID {object_ids[i]}: ({x_meters:.2f}m, {y_meters:.2f}m)"
            cv2.putText(warped_image, text, (int(transformed_center[0]), int(transformed_center[1])), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2, cv2.LINE_AA)

            # If it's the circle marker, draw a circle around it
            if object_ids[i] == CIRCLE_MARKER:
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

def parse_arguments():
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Process a video file to detect and track markers.")
    parser.add_argument("video_path", type=str, help="Path to the input video file")
    parser.add_argument("--debug", type=int, choices=[0, 1, 2, 3], default=0, 
                        help="Set debug level: 0 (off), 1 (basic), 2 (detailed), 3 (save debug frames)")

    return parser.parse_args()

def main():
    try:
        # Parse arguments
        args = parse_arguments()

        # Create output directory if it doesn't exist
        output_dir = os.path.join(os.path.dirname(__file__), 'output_files')
        os.makedirs(output_dir, exist_ok=True)

        # Set debug mode globally
        global debug
        debug = args.debug

        # Debug messages based on the level
        if debug == 0:
            print("Debug mode: Off")
        elif debug == 1:
            print("Debug mode: Basic logging enabled")
        elif debug == 2:
            print("Debug mode: Detailed logging enabled")
        elif debug == 3:
            print("Debug mode: Saving debug frames")

        video_path = args.video_path
        video_name = os.path.splitext(os.path.basename(video_path))[0]
        output_path = os.path.join(output_dir, f'{video_name}_warped_video.mp4')
        output_csv = os.path.join(output_dir, f'{video_name}_marker_positions.csv')

        debug_path = os.path.join(output_dir, f'{video_name}_debug_frame.jpg')

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

        parameters.adaptiveThreshWinSizeMin = 5  # Slightly larger window for better local contrast adaptation
        parameters.adaptiveThreshWinSizeMax = 50  # Larger window to adapt to varying lighting
        parameters.adaptiveThreshConstant = 3  # Increased value to handle distant markers

        parameters.minMarkerPerimeterRate = 0.005  # Detect even smaller markers
        parameters.maxMarkerPerimeterRate = 5.0  # Allow larger markers

        parameters.polygonalApproxAccuracyRate = 0.05  # More relaxed for robustness
        parameters.minCornerDistanceRate = 0.01  # Ensures very small markers don't get filtered out
        parameters.minDistanceToBorder = 0 # Reduce border exclusion to detect near edges

        parameters.markerBorderBits = 1  # Reduces the strictness of border detection
        parameters.minOtsuStdDev = 1.5  # More tolerance to lighting variation
        parameters.perspectiveRemoveIgnoredMarginPerCell = 0.0  # Allows detection of markers at any angle

        parameters.errorCorrectionRate = 0.25  # Low error correction 
        parameters.maxErroneousBitsInBorderRate = 0.8  # Allows detection even if some bits are noisy

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

                detect_markers(detector, frame, valid_ids, tracked_markers)
                selected_corners, selected_ids, object_corners, object_ids = process_markers(tracked_markers, corner_ids)
                if debug >= 2:
                    print(f"Frame {frame_index}: Corner IDs: {selected_ids}")
                    print(f"Frame {frame_index}: Object IDs: {object_ids}")

                if len(selected_corners) == 4:
                    warped_image, matrix = warp_frame(frame, selected_corners, frame_width, frame_height)
                    if warped_image is not None:
                        marker_locations, circle_center = calculate_marker_locations(
                            object_ids, object_corners, matrix, frame_width, frame_height, real_world_distance, radius_meters, warped_image
                        )

                        if circle_center is not None:
                            last_circle_center = circle_center

                        out.write(warped_image)
                        total_processed_frames += 1
                            
                        if CAR_MARKER in marker_locations:
                            x, y = marker_locations[CAR_MARKER]
                            distance_from_circle = np.sqrt((x - last_circle_center[0])**2 + (y - last_circle_center[1])**2) if last_circle_center is not None else 0
                            distance_from_previous_point = np.sqrt((x - previous_x)**2 + (y - previous_y)**2) if previous_x is not None and previous_y is not None else 0
                            speed = distance_from_previous_point / (1 / fps)
                            csv_data.append((fps, frame_index, CAR_MARKER, x, y, distance_from_circle, distance_from_previous_point, speed))

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