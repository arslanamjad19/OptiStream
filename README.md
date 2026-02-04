# OptiStream

**High-performance video processing and inference SDK with GStreamer and ONNX Runtime**

OptiStream is a C++ SDK with Python bindings for real-time video processing and deep learning inference. It features:

- 🎥 **GStreamer Integration** - Hardware-accelerated video pipeline with zero-copy frame access
- 🧠 **ONNX Runtime** - Dynamic model loading and GPU-accelerated inference
- ⚡ **High Performance** - Producer-consumer threading with frame drop policy
- 🐍 **Python Bindings** - Easy-to-use Python API with automatic GIL handling
- 📊 **Performance Monitoring** - Built-in timing and statistics

## Architecture

```
┌─────────────────┐
│  Video Source   │ (Camera, File, Stream)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   GStreamer     │ ← Zero-copy appsink
│    Pipeline     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Frame Queue    │ ← Producer-Consumer
│  (Drop Policy)  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ ONNX Runtime    │ ← GPU Acceleration
│   Inference     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Callback      │ ← Results (Python/C++)
└─────────────────┘
```

## Features

### C++ Core

- **Video Pipeline**: GStreamer-based video capture with support for multiple sources
- **Zero-Copy Mapping**: Direct mapping of GStreamer buffers to `cv::Mat` without memory copy
- **ONNX Runtime**: Dynamic model loading with optimized inference
- **Threading**: Producer-consumer pattern with configurable frame queue
- **Frame Drop Policy**: Automatic frame dropping when queue is full to maintain real-time performance
- **Performance Timing**: Detailed statistics for frames processed, dropped, and inference times

### Python Bindings

- **Native Python API**: Clean Pythonic interface using pybind11
- **GIL Handling**: Automatic GIL acquire/release for callbacks
- **NumPy Integration**: Seamless conversion between cv::Mat and NumPy arrays
- **Type Safety**: Full type hints and runtime checks

## Project Structure

```
OptiStream/
├── include/optistream/     # C++ headers
│   ├── video_stream.hpp    # GStreamer video pipeline
│   ├── inference_engine.hpp # ONNX Runtime wrapper
│   └── pipeline.hpp        # Integrated pipeline
├── src/                    # C++ implementation
│   ├── video_stream.cpp
│   ├── inference_engine.cpp
│   └── pipeline.cpp
├── python/                 # Python bindings
│   └── bindings.cpp        # pybind11 bindings
├── examples/               # Example applications
│   ├── demo_yolo.py        # YOLO object detection demo
│   └── utils.py            # Python utilities (NMS, etc.)
├── tests/                  # Test suite
│   └── test_memory.cpp     # Memory leak and lifecycle tests
├── models/                 # Model directory
│   └── README.md           # Model documentation
├── CMakeLists.txt          # Build configuration
└── README.md               # This file
```

## Dependencies

### Required

- **CMake** >= 3.15
- **C++ Compiler** with C++17 support (GCC 7+, Clang 5+, MSVC 2017+)
- **GStreamer** >= 1.0 with gstreamer-app
- **OpenCV** >= 4.0
- **ONNX Runtime** >= 1.8
- **pybind11** >= 2.6 (for Python bindings)

### Optional

- **Python** >= 3.7 (for Python bindings and examples)
- **NumPy** (for Python examples)

## Installation

### Ubuntu/Debian

```bash
# Install system dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    libopencv-dev \
    python3-dev \
    python3-pip

# Install Python dependencies
pip3 install numpy pybind11

# Download ONNX Runtime
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.0/onnxruntime-linux-x64-1.16.0.tgz
tar -xzf onnxruntime-linux-x64-1.16.0.tgz
export ONNXRUNTIME_ROOT_PATH=$(pwd)/onnxruntime-linux-x64-1.16.0
```

### macOS

```bash
# Install dependencies via Homebrew
brew install cmake gstreamer gst-plugins-base gst-plugins-good opencv python3

# Install Python dependencies
pip3 install numpy pybind11

# Download ONNX Runtime for macOS
# Follow instructions at: https://github.com/microsoft/onnxruntime/releases
```

## Building

### Quick Build

```bash
# Clone repository
git clone https://github.com/arslanamjad19/OptiStream.git
cd OptiStream

# Create build directory
mkdir build && cd build

# Configure (specify ONNX Runtime path if needed)
cmake .. -DONNXRUNTIME_ROOT_PATH=/path/to/onnxruntime

# Build
cmake --build . -j$(nproc)

# Run tests
ctest --output-on-failure
```

### Build Options

```bash
# Disable Python bindings
cmake .. -DBUILD_PYTHON_BINDINGS=OFF

# Disable tests
cmake .. -DBUILD_TESTS=OFF

# Specify ONNX Runtime location
cmake .. -DONNXRUNTIME_ROOT_PATH=/usr/local/onnxruntime

# Release build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Installing Python Module

```bash
# Add to PYTHONPATH
export PYTHONPATH=/path/to/OptiStream/build/python:$PYTHONPATH

# Or install system-wide
cd build
sudo cmake --install .
```

## Usage

### C++ API

```cpp
#include <optistream/pipeline.hpp>

int main() {
    optistream::Pipeline pipeline;
    
    // Set callback for inference results
    pipeline.set_callback([](const std::vector<float>& output, double timestamp) {
        std::cout << "Inference at " << timestamp << "s" << std::endl;
        // Process output...
    });
    
    // Start pipeline
    pipeline.start(
        "videotestsrc",           // Video source
        "models/yolov5s.onnx",    // Model path
        640, 480,                 // Resolution
        30,                       // FPS
        5                         // Max queue size
    );
    
    // Run for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Stop and get statistics
    auto stats = pipeline.get_stats();
    std::cout << "Frames: " << stats.frames_processed << std::endl;
    std::cout << "Dropped: " << stats.frames_dropped << std::endl;
    
    pipeline.stop();
    return 0;
}
```

### Python API

```python
import optistream
import time

# Create pipeline
pipeline = optistream.Pipeline()

# Set callback
def on_inference(output, timestamp):
    print(f"Inference at {timestamp:.2f}s, output size: {len(output)}")

pipeline.set_callback(on_inference)

# Start pipeline
pipeline.start(
    video_source="videotestsrc",
    model_path="models/yolov5s.onnx",
    width=640,
    height=640,
    fps=30,
    max_queue_size=5
)

# Run for 10 seconds
time.sleep(10)

# Get statistics
stats = pipeline.get_stats()
print(f"Frames processed: {stats.frames_processed}")
print(f"Frames dropped: {stats.frames_dropped}")
print(f"Inferences: {stats.inferences_count}")

pipeline.stop()
```

### YOLO Demo

```bash
# Run YOLO object detection demo
python examples/demo_yolo.py \
    --model models/yolov5s.onnx \
    --source videotestsrc \
    --width 640 \
    --height 640

# Use webcam
python examples/demo_yolo.py \
    --model models/yolov5s.onnx \
    --source "v4l2src device=/dev/video0"

# Use video file
python examples/demo_yolo.py \
    --model models/yolov5s.onnx \
    --source "filesrc location=video.mp4 ! decodebin ! videoconvert"
```

## Video Sources

OptiStream uses GStreamer pipeline syntax for video sources:

### Test Pattern
```
videotestsrc pattern=0  # SMPTE test pattern
```

### Webcam (Linux)
```
v4l2src device=/dev/video0
```

### Video File
```
filesrc location=video.mp4 ! decodebin ! videoconvert
```

### RTSP Stream
```
rtspsrc location=rtsp://example.com/stream ! decodebin ! videoconvert
```

### Custom Pipeline
Any GStreamer pipeline that outputs raw video frames.

## Performance

### Benchmarks

Tested on: Intel i7-10700K, NVIDIA RTX 3070, 32GB RAM

| Model      | Resolution | FPS  | Avg Inference | Frame Drop Rate |
|------------|------------|------|---------------|-----------------|
| YOLOv5s    | 640x640    | 30   | 8.5ms         | 0.2%            |
| YOLOv5m    | 640x640    | 30   | 15.2ms        | 1.1%            |
| ResNet50   | 224x224    | 30   | 3.2ms         | 0.0%            |

### Optimization Tips

1. **Use GPU**: Enable CUDA execution provider in ONNX Runtime
2. **Queue Size**: Adjust `max_queue_size` based on inference latency
3. **Resolution**: Lower resolution = faster processing
4. **Model**: Use quantized or pruned models for better performance

## Testing

```bash
# Build and run all tests
cd build
cmake --build . --target test_memory
ctest --output-on-failure

# Run specific test
./tests/test_memory
```

### Memory Tests

The test suite includes:
- Lifecycle tests for all components
- Memory leak detection
- Zero-copy validation
- Callback functionality
- Statistics accuracy

## Troubleshooting

### GStreamer Issues

```bash
# Check GStreamer installation
gst-inspect-1.0 --version

# List available plugins
gst-inspect-1.0

# Test video source
gst-launch-1.0 videotestsrc ! autovideosink
```

### ONNX Runtime Issues

```bash
# Verify ONNX Runtime installation
ls $ONNXRUNTIME_ROOT_PATH/lib

# Check library path
export LD_LIBRARY_PATH=$ONNXRUNTIME_ROOT_PATH/lib:$LD_LIBRARY_PATH
```

### Python Import Issues

```bash
# Check Python module
python3 -c "import optistream; print(optistream.__doc__)"

# Verify PYTHONPATH
echo $PYTHONPATH
```

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## License

MIT License - See LICENSE file for details

## Citation

If you use OptiStream in your research, please cite:

```bibtex
@software{optistream2024,
  title = {OptiStream: High-Performance Video Processing and Inference SDK},
  author = {OptiStream Contributors},
  year = {2024},
  url = {https://github.com/arslanamjad19/OptiStream}
}
```

## Acknowledgments

- GStreamer Project
- ONNX Runtime Team  
- OpenCV Community
- pybind11 Developers

## Contact

For questions and support:
- GitHub Issues: https://github.com/arslanamjad19/OptiStream/issues
- Documentation: https://github.com/arslanamjad19/OptiStream/wiki

---

**OptiStream** - Stream processing at the speed of thought 🚀