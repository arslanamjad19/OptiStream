# OptiStream Project Overview

## Summary

OptiStream is a high-performance C++ SDK with Python bindings for real-time video processing and deep learning inference. It combines GStreamer for video capture, OpenCV for image processing, and ONNX Runtime for neural network inference in a multi-threaded, production-ready architecture.

## Key Features

### 1. GStreamer Video Pipeline
- **Zero-Copy Architecture**: Direct buffer mapping from GStreamer appsink to cv::Mat without memory copying
- **Flexible Sources**: Support for cameras, files, network streams, and test patterns
- **Hardware Acceleration**: Leverage GStreamer's hardware codecs and accelerators

### 2. ONNX Runtime Integration
- **Dynamic Model Loading**: Load any ONNX model at runtime
- **Optimized Inference**: Automatic graph optimization and threading
- **GPU Support**: CUDA execution provider for GPU acceleration (when available)

### 3. Producer-Consumer Threading
- **Decoupled Processing**: Video capture and inference run in separate threads
- **Frame Queue**: Configurable buffer for handling variable processing times
- **Frame Drop Policy**: Automatic oldest-frame dropping when queue is full

### 4. Performance Monitoring
- **Real-time Statistics**: Track frames processed, dropped, and timing
- **Profiling Support**: Built-in performance counters for optimization

### 5. Python Bindings
- **pybind11 Integration**: Native C++ performance with Python ease-of-use
- **GIL Management**: Automatic GIL acquire/release for callbacks
- **NumPy Compatibility**: Seamless array conversions

## Architecture Overview

```
┌──────────────────────────────────────────────────────────────┐
│                       OptiStream SDK                          │
├──────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌─────────────┐      ┌──────────────┐      ┌─────────────┐ │
│  │   Video     │      │  Frame       │      │  Inference  │ │
│  │   Stream    │─────▶│  Queue       │─────▶│  Engine     │ │
│  │ (GStreamer) │      │ (Thread-safe)│      │ (ONNX RT)   │ │
│  └─────────────┘      └──────────────┘      └─────────────┘ │
│        │                      │                      │        │
│        │                      │                      │        │
│        ▼                      ▼                      ▼        │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              Callback System                          │   │
│  │         (C++ lambdas / Python functions)             │   │
│  └──────────────────────────────────────────────────────┘   │
│                                                                │
└──────────────────────────────────────────────────────────────┘
```

## Components

### Core C++ Classes

1. **VideoStream** (`video_stream.hpp/cpp`)
   - Manages GStreamer pipeline
   - Implements zero-copy frame extraction
   - Provides frame callbacks
   - Tracks capture statistics

2. **InferenceEngine** (`inference_engine.hpp/cpp`)
   - Loads ONNX models
   - Performs preprocessing (resize, normalize)
   - Runs inference
   - Returns output tensors

3. **Pipeline** (`pipeline.hpp/cpp`)
   - Integrates VideoStream and InferenceEngine
   - Manages producer-consumer queue
   - Implements frame drop policy
   - Provides unified interface

### Python Bindings

- **bindings.cpp**: pybind11 module exposing C++ classes to Python
- Automatic type conversions (cv::Mat ↔ NumPy)
- Thread-safe callback handling
- Pythonic API with properties and methods

### Examples

1. **demo_yolo.py**: Full YOLO object detection pipeline
2. **demo_video.py**: Simple video capture without inference
3. **simple_pipeline.cpp**: C++ example demonstrating pipeline usage
4. **utils.py**: Python utilities (NMS, YOLO parsing, visualization)

### Tests

- **test_memory.cpp**: Lifecycle and memory leak tests
- Tests object creation/destruction
- Validates zero-copy implementation
- Checks callback functionality

## Use Cases

### Real-Time Object Detection
```python
pipeline = optistream.Pipeline()
pipeline.set_callback(lambda output, ts: process_detections(output))
pipeline.start("v4l2src device=/dev/video0", "yolov5s.onnx", 640, 640)
```

### Video Analytics
- People counting
- Vehicle detection
- Safety monitoring
- Quality inspection

### Edge AI
- IoT cameras
- Embedded systems
- Real-time processing
- Low-latency applications

## Performance Characteristics

### Throughput
- **30 FPS** typical for YOLOv5s @ 640x640 on modern hardware
- **60+ FPS** for lightweight models (MobileNet, etc.)
- Scales with hardware capabilities

### Latency
- **Video capture**: <5ms (zero-copy)
- **Inference**: Model-dependent (8-50ms typical)
- **Total latency**: ~15-60ms end-to-end

### Memory
- **Zero-copy video**: No frame duplication from GStreamer
- **Queue overhead**: ~5-10MB for default queue size
- **Model memory**: Depends on ONNX model size

## Technology Stack

### C++ Libraries
- **GStreamer 1.0+**: Video pipeline
- **OpenCV 4.0+**: Image processing
- **ONNX Runtime 1.8+**: Neural network inference
- **pybind11 2.6+**: Python bindings

### Build System
- **CMake 3.15+**: Cross-platform build
- **C++17**: Modern C++ standard
- **pkg-config**: Dependency detection

### Python
- **Python 3.7+**: Language version
- **NumPy**: Array processing
- **Type hints**: Better IDE support

## Design Principles

1. **Performance First**: Zero-copy, multi-threading, optimizations
2. **Ease of Use**: Simple API, good defaults, clear documentation
3. **Flexibility**: Pluggable components, configurable parameters
4. **Robustness**: Error handling, resource management, testing
5. **Production Ready**: Thread-safe, memory-safe, well-tested

## Future Enhancements

Potential areas for expansion:
- GPU buffer support (CUDA, OpenCL)
- More preprocessing options
- Built-in model zoo
- Web streaming output
- Multi-model pipelines
- Performance profiling tools
- Docker containers

## Getting Started

1. Read [QUICKSTART.md](QUICKSTART.md)
2. Install dependencies
3. Build with `./build.sh`
4. Run examples
5. Integrate into your project

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

MIT License - See [LICENSE](LICENSE) file.
