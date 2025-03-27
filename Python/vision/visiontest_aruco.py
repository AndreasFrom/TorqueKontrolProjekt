import cv2
import numpy as np
import os

def initialize_video(video_path, output_path_video, frame_width, frame_height, fps):
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print("Error: Could not open video.")
        exit()
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_path_video, fourcc, fps, (frame_width, frame_height))
    return cap, out

def detect_markers(detector, frame, valid_ids, tracked_markers):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    corners, ids, _ = detector.detectMarkers(gray)
    detected_ids = set(ids.flatten()) if ids is not None else set()

    if ids is not None:
        for i, marker_id in enumerate(ids.flatten()):
            if marker_id in valid_ids:
                if marker_id not in tracked_markers:
                    tracked_markers[marker_id] = {'corners': corners[i], 'last_known_corners': corners[i]}
                else:
                    tracked_markers[marker_id]['corners'] = corners[i]
                    tracked_markers[marker_id]['last_known_corners'] = corners[i]

    for marker_id in list(tracked_markers.keys()):
        if marker_id not in detected_ids:
            tracked_markers[marker_id]['corners'] = tracked_markers[marker_id]['last_known_corners']

    return corners, ids, detected_ids

def process_corners(tracked_markers, corner_ids):
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

def warp_frame(frame, selected_corners, frame_width, frame_height):
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

def calculate_marker_locations(other_ids, other_corners, matrix, frame_width, frame_height, real_world_distance, radius_meters, warped_image):
    marker_locations = {}
    circle_center = None

    for i in range(len(other_ids)):
        marker_point = other_corners[i][0].reshape(1, 1, -1)
        transformed_center = cv2.perspectiveTransform(marker_point, matrix)[0][0]
        distance_x = (transformed_center[1] / frame_height) * real_world_distance
        distance_y = (transformed_center[0] / frame_width) * real_world_distance
        marker_locations[other_ids[i]] = (distance_x, distance_y)

        text = f"ID {other_ids[i]}: ({distance_x:.2f}m, {distance_y:.2f}m)"
        cv2.putText(warped_image, text, (int(transformed_center[0]), int(transformed_center[1])), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2, cv2.LINE_AA)

        if other_ids[i] == 11:
            radius_pixels = int((radius_meters / real_world_distance) * frame_height)
            cv2.circle(warped_image, (int(transformed_center[0]), int(transformed_center[1])), radius_pixels, (0, 0, 255), 2)
            circle_center = transformed_center

    return marker_locations, circle_center

def main():
    video_path = 'aurco_video.mp4'
    output_dir = os.path.join(os.path.dirname(__file__), 'dataprocessing')
    os.makedirs(output_dir, exist_ok=True)  # Create if not exists
    output_path_video = os.path.join(output_dir, 'output_warped_video.mp4')

    frame_width, frame_height = 400, 400
    real_world_distance = 1
    radius_meters = 0.1
    corner_ids = [6, 7, 8, 9]
    valid_ids = set(range(6, 12))

    cap, out = initialize_video(video_path, output_path_video, frame_width, frame_height, int(cv2.VideoCapture(video_path).get(cv2.CAP_PROP_FPS)))
    aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
    parameters = cv2.aruco.DetectorParameters()
    detector = cv2.aruco.ArucoDetector(aruco_dict, parameters)
    tracked_markers = {}

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        corners, ids, detected_ids = detect_markers(detector, frame, valid_ids, tracked_markers)
        selected_corners, selected_ids, other_corners, other_ids = process_corners(tracked_markers, corner_ids)

        if len(selected_corners) == 4:
            warped_image, matrix = warp_frame(frame, selected_corners, frame_width, frame_height)
            marker_locations, circle_center = calculate_marker_locations(
                other_ids, other_corners, matrix, frame_width, frame_height, real_world_distance, radius_meters, warped_image
            )
            out.write(warped_image)

    cap.release()
    out.release()
    print(f"Warped video saved to {output_path_video}")

if __name__ == "__main__":
    main()
