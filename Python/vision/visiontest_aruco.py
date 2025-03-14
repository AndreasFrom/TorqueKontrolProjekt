import cv2
import numpy as np

# Load the image
image = cv2.imread('aruco_20_10.jpg')

# Convert the image to grayscale
gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
parameters = cv2.aruco.DetectorParameters()

# Create the ArUco detector
detector = cv2.aruco.ArucoDetector(aruco_dict, parameters)
# Detect the markers
corners, ids, rejected = detector.detectMarkers(gray)

# Check if we have detected any markers
if ids is not None:
    print("Detected markers:", ids)

    # Separate the desired markers and other markers
    selected_corners = []
    selected_ids = []
    other_corners = []
    other_ids = []
    corner_ids = [6, 7, 8, 9]  # Define the corner marker IDs

    for i in range(len(ids)):
        if ids[i] in corner_ids:  # Check if the detected ID is in the desired list
            selected_corners.append(corners[i][0])  # Append corners to the list
            selected_ids.append(ids[i])  # Append ID to the list
        else:
            other_corners.append(corners[i][0])  # Append other corners
            other_ids.append(ids[i])  # Append other IDs

    # Draw all detected markers on the original image
    cv2.aruco.drawDetectedMarkers(image, corners, ids)
    cv2.imwrite('image_with_marker_information.jpg', image)

    print("Selected markers:", selected_ids)
    print("Other markers:", other_ids)

    # If we have exactly four selected markers
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
        warped_image = cv2.warpPerspective(image, matrix, (width, height))

        

        # Calculate real-world distances
        real_world_distance = 0.25  # Define the real-world distance (e.g., 1 meter)
        for i in range(len(other_ids)):
            marker_point = other_corners[i][0].reshape(1, 1, -1)  # Get the first corner of the marker
            print(marker_point)
            transformed_center = cv2.perspectiveTransform(marker_point, matrix)[0][0]
            # Calculate distances in the real-world coordinate system
            distance_x = (transformed_center[1] / height) * real_world_distance
            distance_y = (transformed_center[0] / width) * real_world_distance
            print(f"Marker ID {other_ids[i]} location: ({distance_x} meters, {distance_y} meters)")

            # Write the marker ID and location on the warped image
            text = f"ID {other_ids[i]}: ({distance_x:.2f}m, {distance_y:.2f}m)"
            cv2.putText(warped_image, text, (int(transformed_center[0]), int(transformed_center[1])), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2, cv2.LINE_AA)
            
        # Save and display the warped image with other marker locations
        cv2.imwrite('warped_image_with_other_markers.jpg', warped_image)    
    else:
        print("Not enough selected markers detected.")
else:
    print("No markers detected.")