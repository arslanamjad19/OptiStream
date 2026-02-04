#!/usr/bin/env python3
"""
Simple video capture example using OptiStream
Demonstrates basic video streaming without inference
"""

import sys
import time
import argparse

try:
    import optistream
    import numpy as np
except ImportError as e:
    print(f"Error: {e}")
    print("Please install required packages and build OptiStream")
    sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="OptiStream Video Capture Demo")
    parser.add_argument(
        "--source",
        type=str,
        default="videotestsrc pattern=0",
        help="Video source (default: videotestsrc pattern=0)"
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
        default=480,
        help="Frame height (default: 480)"
    )
    parser.add_argument(
        "--fps",
        type=int,
        default=30,
        help="Frames per second (default: 30)"
    )
    parser.add_argument(
        "--duration",
        type=int,
        default=5,
        help="Run duration in seconds (default: 5)"
    )
    
    args = parser.parse_args()
    
    print("OptiStream Video Capture Demo")
    print("=" * 50)
    print(f"Source: {args.source}")
    print(f"Resolution: {args.width}x{args.height}")
    print(f"FPS: {args.fps}")
    print(f"Duration: {args.duration}s")
    print()
    
    # Create video stream
    stream = optistream.VideoStream()
    
    frame_count = 0
    
    def on_frame(frame, timestamp):
        nonlocal frame_count
        frame_count += 1
        
        if frame_count == 1:
            print(f"First frame received:")
            print(f"  Shape: {frame.shape}")
            print(f"  Timestamp: {timestamp:.3f}s")
            print()
        
        if frame_count % 30 == 0:
            print(f"Processed {frame_count} frames @ {timestamp:.2f}s")
    
    # Set callback
    stream.set_callback(on_frame)
    
    # Start stream
    if not stream.start(args.source, args.width, args.height, args.fps):
        print("Failed to start video stream")
        return 1
    
    print("Stream started. Capturing frames...")
    print()
    
    # Run for specified duration
    try:
        time.sleep(args.duration)
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    
    # Stop stream
    stream.stop()
    
    # Get and print statistics
    stats = stream.get_stats()
    print()
    print("=" * 50)
    print("Statistics:")
    print(f"  Frames processed: {stats.frames_processed}")
    print(f"  Frames dropped: {stats.frames_dropped}")
    print(f"  Avg processing time: {stats.avg_processing_time_ms:.2f} ms")
    print(f"  Callback received: {frame_count} frames")
    
    if stats.frames_processed > 0:
        drop_rate = (stats.frames_dropped / stats.frames_processed) * 100
        print(f"  Drop rate: {drop_rate:.2f}%")
        print(f"  Expected FPS: {args.fps}")
        print(f"  Actual FPS: {stats.frames_processed / args.duration:.1f}")
    
    print()
    print("Demo completed successfully!")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
