cmake_minimum_required(VERSION 3.15)
project(OptiStream VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build C++ examples" ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# GStreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)

# OpenCV
find_package(OpenCV REQUIRED)

# ONNX Runtime
# Note: ONNX Runtime may need to be installed separately
# For Ubuntu: download from https://github.com/microsoft/onnxruntime/releases
set(ONNXRUNTIME_ROOT_PATH "" CACHE PATH "Path to ONNX Runtime installation")
if(ONNXRUNTIME_ROOT_PATH)
    set(ONNXRUNTIME_INCLUDE_DIRS "${ONNXRUNTIME_ROOT_PATH}/include")
    set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT_PATH}/lib")
    find_library(ONNXRUNTIME_LIBRARY onnxruntime PATHS ${ONNXRUNTIME_LIB_DIR})
else()
    find_path(ONNXRUNTIME_INCLUDE_DIRS onnxruntime_cxx_api.h
        PATHS /usr/include /usr/local/include
        PATH_SUFFIXES onnxruntime onnxruntime/core/session)
    find_library(ONNXRUNTIME_LIBRARY onnxruntime
        PATHS /usr/lib /usr/local/lib)
endif()

if(NOT ONNXRUNTIME_LIBRARY)
    message(WARNING "ONNX Runtime not found. Set ONNXRUNTIME_ROOT_PATH or install ONNX Runtime.")
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_INCLUDE_DIRS}
)

# Link directories
link_directories(
    ${GSTREAMER_LIBRARY_DIRS}
    ${GSTREAMER_APP_LIBRARY_DIRS}
)

# Source files
set(OPTISTREAM_SOURCES
    src/video_stream.cpp
    src/inference_engine.cpp
    src/pipeline.cpp
)

# Create shared library
add_library(optistream_core SHARED ${OPTISTREAM_SOURCES})

target_link_libraries(optistream_core
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${OpenCV_LIBS}
    ${ONNXRUNTIME_LIBRARY}
)

# Python bindings
if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)
    
    if(pybind11_FOUND)
        pybind11_add_module(optistream python/bindings.cpp)
        target_link_libraries(optistream PRIVATE optistream_core)
        
        # Install Python module
        install(TARGETS optistream
            LIBRARY DESTINATION python)
    else()
        message(WARNING "pybind11 not found. Python bindings will not be built.")
        message(STATUS "Install pybind11: pip install pybind11")
    endif()
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_executable(simple_pipeline examples/simple_pipeline.cpp)
    target_link_libraries(simple_pipeline
        optistream_core
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_APP_LIBRARIES}
        ${OpenCV_LIBS}
    )
    
    install(TARGETS simple_pipeline
        RUNTIME DESTINATION bin)
endif()

# Install
install(TARGETS optistream_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/optistream
    DESTINATION include
)

# Print configuration summary
message(STATUS "")
message(STATUS "OptiStream Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Python bindings: ${BUILD_PYTHON_BINDINGS}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  GStreamer: ${GSTREAMER_VERSION}")
message(STATUS "  OpenCV: ${OpenCV_VERSION}")
if(ONNXRUNTIME_LIBRARY)
    message(STATUS "  ONNX Runtime: ${ONNXRUNTIME_LIBRARY}")
else()
    message(STATUS "  ONNX Runtime: NOT FOUND")
endif()
if(pybind11_FOUND)
    message(STATUS "  pybind11: ${pybind11_VERSION}")
else()
    message(STATUS "  pybind11: NOT FOUND")
endif()
message(STATUS "")
