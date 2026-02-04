#!/usr/bin/env python3
"""
OptiStream YOLO Demo
Demonstrates real-time object detection using OptiStream SDK

Usage:
    python demo_yolo.py --model models/yolov5s.onnx --source videotestsrc
    python demo_yolo.py --model models/yolov5s.onnx --source v4l2src device=/dev/video0
    python demo_yolo.py --model models/yolov5s.onnx --source filesrc location=video.mp4
"""

import sys
import time
import argparse
import numpy as np
import cv2

# Import OptiStream (ensure it's built and in PYTHONPATH)
try:
    import optistream
except ImportError:
    print("Error: OptiStream module not found.")
    print("Please build the project and ensure the Python module is in PYTHONPATH")
    print("Example: export PYTHONPATH=/path/to/build/python:$PYTHONPATH")
    sys.exit(1)

from utils import parse_yolo_output, draw_boxes, COCO_CLASSES


class YOLODetector:
    """YOLO object detector using OptiStream"""
    
    def __init__(self, model_path, video_source, width=640, height=640):
        self.model_path = model_path
        self.video_source = video_source
        self.width = width
        self.height = height
        self.pipeline = optistream.Pipeline()
        self.detections = []
        self.latest_frame = None
        self.running = False
        
    def inference_callback(self, output, timestamp):
        """Callback for inference results"""
        # Convert output to numpy array
        output_np = np.array(output, dtype=np.float32)
        
        # Parse YOLO output
        boxes, scores, class_ids = parse_yolo_output(
            output_np, 
            conf_threshold=0.25,
            iou_threshold=0.45,
            img_width=self.width,
            img_height=self.height
        )
        
        # Store detections
        self.detections = (boxes, scores, class_ids, timestamp)
        
        # Print detections
        if len(boxes) > 0:
            print(f"[{timestamp:.2f}s] Detected {len(boxes)} objects:")
            for box, score, class_id in zip(boxes, scores, class_ids):
                class_name = COCO_CLASSES[class_id] if class_id < len(COCO_CLASSES) else f"Class{class_id}"
                print(f"  - {class_name}: {score:.2f}")
    
    def start(self):
        """Start the detection pipeline"""
        print(f"Starting YOLO detector...")
        print(f"  Model: {self.model_path}")
        print(f"  Source: {self.video_source}")
        print(f"  Resolution: {self.width}x{self.height}")
        print()
        
        # Set callback
        self.pipeline.set_callback(self.inference_callback)
        
        # Start pipeline
        if not self.pipeline.start(
            self.video_source,
            self.model_path,
            self.width,
            self.height,
            fps=30,
            max_queue_size=5
        ):
            print("Failed to start pipeline")
            return False
        
        self.running = True
        print("Pipeline started successfully")
        print("Press Ctrl+C to stop")
        print()
        return True
    
    def stop(self):
        """Stop the detection pipeline"""
        if self.running:
            print("\nStopping pipeline...")
            self.pipeline.stop()
            self.running = False
            
            # Print statistics
            stats = self.pipeline.get_stats()
            print("\nPipeline Statistics:")
            print(f"  Frames processed: {stats.frames_processed}")
            print(f"  Frames dropped: {stats.frames_dropped}")
            print(f"  Inferences: {stats.inferences_count}")
            print(f"  Avg frame time: {stats.avg_frame_time_ms:.2f} ms")
            print(f"  Avg inference time: {stats.avg_inference_time_ms:.2f} ms")
            
            if stats.frames_processed > 0:
                drop_rate = (stats.frames_dropped / stats.frames_processed) * 100
                print(f"  Frame drop rate: {drop_rate:.2f}%")
    
    def run(self, duration=None):
        """Run the detector for a specified duration"""
        if not self.start():
            return
        
        try:
            start_time = time.time()
            while self.running:
                time.sleep(0.1)
                
                # Check duration
                if duration and (time.time() - start_time) > duration:
                    break
                    
        except KeyboardInterrupt:
            print("\nInterrupted by user")
        finally:
            self.stop()


def main():
    parser = argparse.ArgumentParser(description="OptiStream YOLO Demo")
    parser.add_argument(
        "--model",
        type=str,
        required=True,
        help="Path to YOLO ONNX model (e.g., models/yolov5s.onnx)"
    )
    parser.add_argument(
        "--source",
        type=str,
        default="videotestsrc",
        help="Video source (e.g., videotestsrc, v4l2src device=/dev/video0, filesrc location=video.mp4)"
    )
    parser.add_argument(
        "--width",
        type=int,
        default=640,
        help="Frame width (default: 640)"
    )
    parser.add_argument(
        "--height",
        type=int,
        default=640,
        help="Frame height (default: 640)"
    )
    parser.add_argument(
        "--duration",
        type=int,
        default=None,
        help="Run duration in seconds (default: run until interrupted)"
    )
    
    args = parser.parse_args()
    
    # Check if model exists
    import os
    if not os.path.exists(args.model):
        print(f"Error: Model file not found: {args.model}")
        print("\nTo use this demo, you need a YOLO ONNX model.")
        print("You can download YOLOv5 models from:")
        print("  https://github.com/ultralytics/yolov5")
        print("\nOr export your own model to ONNX format:")
        print("  pip install yolov5")
        print("  yolo export model=yolov5s.pt format=onnx")
        sys.exit(1)
    
    # Create and run detector
    detector = YOLODetector(
        args.model,
        args.source,
        args.width,
        args.height
    )
    
    detector.run(args.duration)


if __name__ == "__main__":
    main()
