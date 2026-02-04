#!/bin/bash
# Build script for OptiStream SDK

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== OptiStream Build Script ===${NC}"
echo

# Parse arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_TESTS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --debug      Build in Debug mode (default: Release)"
            echo "  --clean      Clean build directory before building"
            echo "  --test       Run tests after building"
            echo "  --help       Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

# Configure
echo -e "${GREEN}Configuring CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DBUILD_PYTHON_BINDINGS=ON \
    -DBUILD_TESTS=ON \
    || { echo -e "${RED}CMake configuration failed${NC}"; exit 1; }

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . -j$(nproc) \
    || { echo -e "${RED}Build failed${NC}"; exit 1; }

echo -e "${GREEN}Build completed successfully!${NC}"
echo

# Print build info
echo "Build Information:"
echo "  Type: $BUILD_TYPE"
echo "  Library: $(ls -lh liboptistream_core.so 2>/dev/null | awk '{print $9, "("$5")")' || echo "Not found")"
if [ -f python/optistream*.so ]; then
    echo "  Python module: $(ls python/optistream*.so)"
fi
echo

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    echo -e "${GREEN}Running tests...${NC}"
    ctest --output-on-failure \
        || { echo -e "${RED}Tests failed${NC}"; exit 1; }
    echo -e "${GREEN}All tests passed!${NC}"
    echo
fi

# Print usage instructions
echo -e "${GREEN}=== Build Complete ===${NC}"
echo
echo "To use the Python module:"
echo "  export PYTHONPATH=$(pwd)/python:\$PYTHONPATH"
echo
echo "To run the C++ test:"
echo "  ./tests/test_memory"
echo
echo "To run the Python demo (requires ONNX model):"
echo "  python3 ../examples/demo_yolo.py --model ../models/yolov5s.onnx --source videotestsrc"
echo
