import os
import subprocess

video_folder = "../../Datasets/b5-1/"

def process_video(video_file):
    """Runs analyze_video.py on a single video file."""
    video_path = os.path.join(video_folder, video_file)
    subprocess.run(["python", "analyze_video.py", video_path, "--debug", "2"])

if __name__ == "__main__":
    video_files = [f for f in os.listdir(video_folder) if f.endswith((".mov", ".mp4"))]

    # Process videos one at a time (sequentially)
    for video_file in video_files:
        process_video(video_file)

    # Run vision_data_plot.py after all videos are processed
    subprocess.run(["python", "repair_data.py"], check=True)
    subprocess.run(["python", "vision_data_plot.py"], check=True)
