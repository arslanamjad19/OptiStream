# CI/CD Configuration Template

This directory contains templates for continuous integration and deployment.

## GitHub Actions Template

Create `.github/workflows/build.yml` in your repository with:

```yaml
name: Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build-ubuntu:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          build-essential cmake \
          libgstreamer1.0-dev \
          libgstreamer-plugins-base1.0-dev \
          gstreamer1.0-plugins-base \
          gstreamer1.0-plugins-good \
          libopencv-dev \
          python3-dev python3-pip
        pip3 install numpy pybind11
    
    - name: Download ONNX Runtime
      run: |
        wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.0/onnxruntime-linux-x64-1.16.0.tgz
        tar -xzf onnxruntime-linux-x64-1.16.0.tgz
    
    - name: Build and Test
      run: ./build.sh --test
```

## Docker Template

Create `Dockerfile` in your repository root with:

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libopencv-dev \
    python3-dev python3-pip wget

RUN pip3 install numpy pybind11

RUN wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.0/onnxruntime-linux-x64-1.16.0.tgz && \
    tar -xzf onnxruntime-linux-x64-1.16.0.tgz && \
    mv onnxruntime-linux-x64-1.16.0 /opt/onnxruntime

COPY . /app
WORKDIR /app

RUN ./build.sh

ENV LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH
ENV PYTHONPATH=/app/build/python:$PYTHONPATH

CMD ["/bin/bash"]
```
