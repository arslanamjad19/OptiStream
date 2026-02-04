# OptiStream Quick Start Guide

This guide will help you get OptiStream up and running quickly.

## Prerequisites

Before building OptiStream, ensure you have the following installed:

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    libopencv-dev \
    python3-dev \
    python3-pip

pip3 install numpy pybind11
```

### ONNX Runtime

Download and extract ONNX Runtime:
```bash
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.0/onnxruntime-linux-x64-1.16.0.tgz
tar -xzf onnxruntime-linux-x64-1.16.0.tgz
export ONNXRUNTIME_ROOT_PATH=$(pwd)/onnxruntime-linux-x64-1.16.0
```

## Building

### Quick Build
```bash
./build.sh
```

### Build with Options
```bash
# Debug build
./build.sh --debug

# Clean build
./build.sh --clean

# Build and run tests
./build.sh --test
```

### Manual Build
```bash
mkdir build && cd build
cmake .. -DONNXRUNTIME_ROOT_PATH=/path/to/onnxruntime
cmake --build . -j$(nproc)
ctest --output-on-failure
```

## Running Examples

### 1. Setup Python Path
```bash
export PYTHONPATH=$(pwd)/build/python:$PYTHONPATH
```

### 2. Verify Installation
```bash
python3 setup_python.py
```

### 3. Test with Video Test Source
```bash
# First, test GStreamer
gst-launch-1.0 videotestsrc ! autovideosink

# Run C++ test
./build/tests/test_memory
```

### 4. Run Python Demo (requires ONNX model)

Download a YOLO model:
```bash
# Using YOLOv5
pip install ultralytics
yolo export model=yolov5s.pt format=onnx
mv yolov5s.onnx models/
```

Run the demo:
```bash
python examples/demo_yolo.py \
    --model models/yolov5s.onnx \
    --source videotestsrc \
    --width 640 \
    --height 640
```

## Common Issues

### GStreamer not found
```bash
# Verify installation
gst-inspect-1.0 --version
pkg-config --modversion gstreamer-1.0
```

### ONNX Runtime not found
```bash
# Set the path explicitly
export ONNXRUNTIME_ROOT_PATH=/path/to/onnxruntime
export LD_LIBRARY_PATH=$ONNXRUNTIME_ROOT_PATH/lib:$LD_LIBRARY_PATH
```

### Python module not found
```bash
# Add to Python path
export PYTHONPATH=/path/to/OptiStream/build/python:$PYTHONPATH

# Verify
python3 -c "import optistream; print('Success!')"
```

### OpenCV not found
```bash
# Install OpenCV
sudo apt-get install libopencv-dev

# Or build from source
# https://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html
```

## Next Steps

1. Read the full [README.md](README.md) for detailed documentation
2. Explore [examples/](examples/) for more use cases
3. Check [tests/](tests/) for test examples
4. See [models/README.md](models/README.md) for model information

## Getting Help

- Check the [README.md](README.md) troubleshooting section
- Open an issue on GitHub
- Review the example code in `examples/`

## Quick Reference

### Video Sources
```bash
# Test pattern
videotestsrc

# Webcam (Linux)
v4l2src device=/dev/video0

# Video file
filesrc location=video.mp4 ! decodebin ! videoconvert

# RTSP stream
rtspsrc location=rtsp://example.com/stream ! decodebin ! videoconvert
```

### Build Targets
```bash
cmake --build . --target optistream_core  # C++ library only
cmake --build . --target optistream       # Python module
cmake --build . --target test_memory      # Tests
```

### Useful Commands
```bash
# Check GStreamer elements
gst-inspect-1.0 | grep -i video

# Test video pipeline
gst-launch-1.0 videotestsrc ! videoconvert ! autovideosink

# Check library dependencies
ldd build/liboptistream_core.so

# View Python module info
python3 -c "import optistream; help(optistream)"
```
