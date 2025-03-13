import cv2
import numpy as np

# Load the image
image = cv2.imread('aruco_car_30.jpg')

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

    # Let's assume you want to work with the first four marker corners.
    selected_corners = []
    selected_ids = []
    # Define the desired order of marker IDs
    desired_ids = [6, 7, 8, 11]
    
    for i in range(len(ids)):
        if ids[i] in desired_ids:  # Check if the detected ID is in the desired order
            selected_corners.append(corners[i][0])  # Append corners to the list
            selected_ids.append(ids[i])  # Append ID to the list
    
    
    # Draw the detected markers
    cv2.aruco.drawDetectedMarkers(image, corners, ids)
    cv2.imwrite('image_with_marker_information.jpg', image)

    print("Detected markers:", selected_ids)
    print("Detected corners:", selected_corners)

    # If we have exactly four selected markers (adjust based on your needs)
    if len(selected_corners) == 4:
        # Flatten the list of corners (four lists of 4 points)
        points = np.array([corner[0] for corner in selected_corners], dtype=np.float32)
        print("Points:", points)   

        # Sort the points based on their x-coordinates
        sorted_points = points[np.argsort(points[:, 0])]

        # Determine the top-left and bottom-left points (smallest x-coordinates)
        left_points = sorted_points[:2]
        # Determine the top-right and bottom-right points (largest x-coordinates)
        right_points = sorted_points[2:]

        # Sort the left points based on their y-coordinates to get top-left and bottom-left
        left_points = left_points[np.argsort(left_points[:, 1])]
        top_left, bottom_left = left_points

        # Sort the right points based on their y-coordinates to get top-right and bottom-right
        right_points = right_points[np.argsort(right_points[:, 1])]
        top_right, bottom_right = right_points

        # Arrange the points in the correct order: top-left, top-right, bottom-right, bottom-left
        ordered_points = np.array([top_left, top_right, bottom_right, bottom_left], dtype=np.float32)

        # Define the destination points in the output image (this is where you'll map the corners)
        width, height = 600, 600  # Define the output image size
        dst_points = np.array([[0, 0], [width-1, 0], [width-1, height-1], [0, height-1]], dtype=np.float32)

        # Get the perspective transform matrix
        matrix = cv2.getPerspectiveTransform(ordered_points, dst_points)

        # Apply the perspective transformation
        warped_image = cv2.warpPerspective(image, matrix, (width, height))

        # Show the warped image
        cv2.imwrite('Warped_image.jpg', warped_image)
        
    else:
        print("Not enough markers detected.")
else:
    print("No markers detected.")
