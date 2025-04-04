import os
import subprocess
from concurrent.futures import ProcessPoolExecutor

video_folder = "../../Datasets/04_04_2025/" 

def process_video(video_file):
    """Runs analyze_video.py on a single video file."""
    video_path = os.path.join(video_folder, video_file)
    result = subprocess.run(["python", "analyze_video.py", video_path], capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"Error processing {video_file}: {result.stderr}")
    else:
        print(f"Successfully processed {video_file}")

if __name__ == "__main__":
    video_files = [f for f in os.listdir(video_folder) if f.endswith((".mov", ".mp4"))]

    # Run video analysis in parallel using ProcessPoolExecutor
    with ProcessPoolExecutor() as executor:
        executor.map(process_video, video_files)

    # Run vision_data_plot.py after all videos are processed
    subprocess.run(["python", "vision_data_plot.py"], check=True)
