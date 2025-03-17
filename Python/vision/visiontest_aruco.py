import cv2
import numpy as np

# Open the video file
video_path = 'aurco_video.mp4'
cap = cv2.VideoCapture(video_path)

# Check if the video was opened successfully
if not cap.isOpened():
    print("Error: Could not open video.")
    exit()

# Define the ArUco dictionary and parameters
aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
parameters = cv2.aruco.DetectorParameters()
detector = cv2.aruco.ArucoDetector(aruco_dict, parameters)

# Define the marker IDs (6 to 11)
valid_ids = set(range(6, 12))  # IDs 6, 7, 8, 9, 10, 11
corner_ids = [6, 7, 8, 9]  # IDs for the corners
tracked_markers = {}  # Dictionary to store marker IDs and their last known positions

# Define the real-world distance and radius
real_world_distance = 1  # Real-world distance in meters
radius_meters = 0.1  # Radius of the circle in meters

# Get video properties for VideoWriter
frame_width = 400  # Width of the warped image
frame_height = 400  # Height of the warped image
fps = int(cap.get(cv2.CAP_PROP_FPS))

# Define the output video file
output_path = 'output_warped_video.mp4'
fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # Codec for MP4 format
out = cv2.VideoWriter(output_path, fourcc, fps, (frame_width, frame_height))

# Loop through each frame of the video
while True:
    # Read a frame from the video
    ret, frame = cap.read()
    if not ret:
        break  # Exit the loop if no more frames are available

    # Convert the frame to grayscale
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Detect the markers
    corners, ids, rejected = detector.detectMarkers(gray)

    # Create a set of detected IDs in the current frame
    detected_ids = set()
    if ids is not None:
        detected_ids = set(ids.flatten())  # Flatten the array of IDs

    # Update the dictionary with the new IDs and corners (only for valid IDs)
    if ids is not None:
        for i, marker_id in enumerate(ids.flatten()):
            if marker_id in valid_ids:  # Only process valid IDs (6 to 11)
                if marker_id not in tracked_markers:
                    # Add new marker to the dictionary
                    tracked_markers[marker_id] = {
                        'corners': corners[i],
                        'last_known_corners': corners[i]  # Initialize last known position
                    }
                else:
                    # Update the marker's current and last known positions
                    tracked_markers[marker_id]['corners'] = corners[i]
                    tracked_markers[marker_id]['last_known_corners'] = corners[i]

    # Handle markers that are not detected in the current frame
    for marker_id in list(tracked_markers.keys()):
        if marker_id not in detected_ids:
            # If the marker is not detected, use its last known position
            tracked_markers[marker_id]['corners'] = tracked_markers[marker_id]['last_known_corners']

    # Check if we have detected any markers
    if ids is not None:
        # Separate the desired markers and other markers
        selected_corners = []
        selected_ids = []
        other_corners = []
        other_ids = []

        for marker_id, marker_data in tracked_markers.items():
            if marker_id in corner_ids:  # Check if the detected ID is in the corner list
                selected_corners.append(marker_data['corners'][0])  # Append corners to the list
                selected_ids.append(marker_id)  # Append ID to the list
            elif marker_id in valid_ids:  # Only process valid IDs (6 to 11)
                other_corners.append(marker_data['corners'][0])  # Append other corners
                other_ids.append(marker_id)  # Append other IDs

        # If we have exactly four selected markers (corners)
        if len(selected_corners) == 4:
            # Flatten the list of corners (four lists of 4 points)
            points = np.array([corner for corner in selected_corners], dtype=np.float32)

            # Sort the points to ensure correct order: top-left, top-right, bottom-right, bottom-left
            sorted_points = points[np.argsort(points[:, 0, 0])]  # Sort by x-coordinate
            left_points = sorted_points[:2]  # Two leftmost points
            right_points = sorted_points[2:]  # Two rightmost points

            # Sort left and right points by y-coordinate to get top and bottom
            left_points = left_points[np.argsort(left_points[:, 0, 1])]
            right_points = right_points[np.argsort(right_points[:, 0, 1])]
            top_left, bottom_left = left_points
            top_right, bottom_right = right_points

            # Arrange the points in the correct order
            ordered_points = np.array([top_left[0], top_right[0], bottom_right[0], bottom_left[0]], dtype=np.float32)

            # Define the destination points in the output image
            width, height = 400, 400  # Define the output image size
            dst_points = np.array([[0, 0], [width-1, 0], [width-1, height-1], [0, height-1]], dtype=np.float32)

            # Get the perspective transform matrix
            matrix = cv2.getPerspectiveTransform(ordered_points, dst_points)

            # Apply the perspective transformation
            warped_image = cv2.warpPerspective(frame, matrix, (width, height))

            # Calculate real-world distances
            marker_locations = {}  # Dictionary to store marker locations

            for i in range(len(other_ids)):
                marker_point = other_corners[i][0].reshape(1, 1, -1)  # Get the first corner of the marker
                transformed_center = cv2.perspectiveTransform(marker_point, matrix)[0][0]
                # Calculate distances in the real-world coordinate system
                distance_x = (transformed_center[1] / height) * real_world_distance
                distance_y = (transformed_center[0] / width) * real_world_distance
                marker_locations[other_ids[i]] = (distance_x, distance_y)
                print(f"Marker ID {other_ids[i]} location: ({distance_x} meters, {distance_y} meters)")

                # Convert radius to pixels
                radius_pixels = int((radius_meters / real_world_distance) * height)

                # Write the marker ID and location on the warped image
                text = f"ID {other_ids[i]}: ({distance_x:.2f}m, {distance_y:.2f}m)"
                cv2.putText(warped_image, text, (int(transformed_center[0]), int(transformed_center[1])), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2, cv2.LINE_AA)

                # Draw a circle if the marker ID is 11
                if other_ids[i] == 11:
                    cv2.circle(warped_image, (int(transformed_center[0]), int(transformed_center[1])), radius_pixels, (0, 0, 255), 2)
                    circle_center = transformed_center  # Save the circle center for distance calculation

            # Calculate distances of other points to the closest point on the circle perimeter
            if 'circle_center' in locals():  # Check if circle_center is defined
                for i in range(len(other_ids)):
                    if other_ids[i] != 11:  # Skip the circle marker itself
                        marker_point = other_corners[i][0].reshape(1, 1, -1)
                        transformed_center = cv2.perspectiveTransform(marker_point, matrix)[0][0]
                        
                        # Calculate distance to the circle center in meters
                        distance_to_center_x = (transformed_center[1] / height) * real_world_distance - (circle_center[1] / height) * real_world_distance
                        distance_to_center_y = (transformed_center[0] / width) * real_world_distance - (circle_center[0] / width) * real_world_distance
                        distance_to_center = np.sqrt(distance_to_center_x**2 + distance_to_center_y**2)
                        
                        # Calculate distance to the circle perimeter in meters
                        radius_meters = (radius_pixels / height) * real_world_distance
                        distance_to_perimeter = distance_to_center - radius_meters  # Subtract the radius to get the distance to the perimeter
                        
                        # Print or use the distance_to_perimeter as needed
                        print(f"Distance to perimeter for marker {other_ids[i]}: {distance_to_perimeter} meters")
            else:
                print("Circle center not defined. Marker ID 11 not detected in this frame.")

            # Write the warped frame to the output video
            out.write(warped_image)

# Release the video capture and writer objects
cap.release()
out.release()
print(f"Warped video saved to {output_path}")