import os
import subprocess
import threading

video_folder = "../../Datasets/a7-1/"

def process_video(video_file):
    """Runs analyze_video.py on a single video file."""
    video_path = os.path.join(video_folder, video_file)
    subprocess.run(["python", "analyze_video.py", video_path, "--debug", "2"])

def process_videos_multithreaded(video_files):
    threads = []
    for video_file in video_files:
        t = threading.Thread(target=process_video, args=(video_file,))
        t.start()
        threads.append(t)
    for t in threads:
        t.join()

if __name__ == "__main__":
    video_files = [f for f in os.listdir(video_folder) if f.endswith((".mov", ".mp4"))]

    # Process videos one at a time (sequentially)
    process_videos_multithreaded(video_files)

    # Run vision_data_plot.py after all videos are processed
    subprocess.run(["python", "repair_data.py"], check=True)
    subprocess.run(["python", "vision_data_plot.py"], check=True)
    subprocess.run(["python", "3dplot_position.py"], check=True)
    subprocess.run(["python", "x_y_pointplot.py"], check=True)
    subprocess.run(["python", "ekstremum.py"], check=True)
