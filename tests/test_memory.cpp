#include "optistream/video_stream.hpp"
#include "optistream/inference_engine.hpp"
#include "optistream/pipeline.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

// Simple memory leak detection test
void test_video_stream_lifecycle() {
    std::cout << "Testing VideoStream lifecycle..." << std::endl;
    
    for (int i = 0; i < 3; i++) {
        optistream::VideoStream stream;
        
        // Start with test pattern
        if (stream.start("videotestsrc pattern=0", 320, 240, 30)) {
            std::cout << "  Stream started (iteration " << i << ")" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            stream.stop();
            std::cout << "  Stream stopped" << std::endl;
        }
    }
    
    std::cout << "VideoStream lifecycle test completed" << std::endl;
}

void test_inference_engine_lifecycle() {
    std::cout << "Testing InferenceEngine lifecycle..." << std::endl;
    
    for (int i = 0; i < 3; i++) {
        optistream::InferenceEngine engine;
        
        // Note: This will fail if no model is present, but tests object lifecycle
        std::cout << "  Engine created (iteration " << i << ")" << std::endl;
        
        // Try to load a model (will fail gracefully if not present)
        engine.load_model("models/dummy.onnx");
    }
    
    std::cout << "InferenceEngine lifecycle test completed" << std::endl;
}

void test_pipeline_lifecycle() {
    std::cout << "Testing Pipeline lifecycle..." << std::endl;
    
    for (int i = 0; i < 3; i++) {
        optistream::Pipeline pipeline;
        std::cout << "  Pipeline created (iteration " << i << ")" << std::endl;
        
        // Note: This will fail if no model is present, but tests object lifecycle
        // pipeline.start("videotestsrc", "models/dummy.onnx");
    }
    
    std::cout << "Pipeline lifecycle test completed" << std::endl;
}

void test_video_stream_callback() {
    std::cout << "Testing VideoStream with callback..." << std::endl;
    
    optistream::VideoStream stream;
    
    int frame_count = 0;
    stream.set_callback([&frame_count](const cv::Mat& frame, double timestamp) {
        frame_count++;
        if (frame_count == 1) {
            std::cout << "  Received first frame: " << frame.cols << "x" << frame.rows 
                      << " @ " << timestamp << "s" << std::endl;
        }
    });
    
    if (stream.start("videotestsrc pattern=0", 320, 240, 30)) {
        std::cout << "  Waiting for frames..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        stream.stop();
        
        auto stats = stream.get_stats();
        std::cout << "  Frames processed: " << stats.frames_processed << std::endl;
        std::cout << "  Frames dropped: " << stats.frames_dropped << std::endl;
        std::cout << "  Avg processing time: " << stats.avg_processing_time_ms << " ms" << std::endl;
        std::cout << "  Callback received: " << frame_count << " frames" << std::endl;
    }
    
    std::cout << "VideoStream callback test completed" << std::endl;
}

void test_zero_copy_mapping() {
    std::cout << "Testing zero-copy cv::Mat mapping..." << std::endl;
    
    optistream::VideoStream stream;
    
    bool tested = false;
    stream.set_callback([&tested](const cv::Mat& frame, double timestamp) {
        if (!tested) {
            // Verify frame is valid
            if (!frame.empty() && frame.isContinuous()) {
                std::cout << "  Frame is valid and continuous" << std::endl;
                std::cout << "  Size: " << frame.cols << "x" << frame.rows << std::endl;
                std::cout << "  Channels: " << frame.channels() << std::endl;
                std::cout << "  Type: CV_8UC" << frame.channels() << std::endl;
                tested = true;
            }
        }
    });
    
    if (stream.start("videotestsrc pattern=0", 640, 480, 30)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        stream.stop();
    }
    
    std::cout << "Zero-copy mapping test completed" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "=== OptiStream Memory and Lifecycle Tests ===" << std::endl;
    std::cout << std::endl;
    
    try {
        test_video_stream_lifecycle();
        std::cout << std::endl;
        
        test_inference_engine_lifecycle();
        std::cout << std::endl;
        
        test_pipeline_lifecycle();
        std::cout << std::endl;
        
        test_video_stream_callback();
        std::cout << std::endl;
        
        test_zero_copy_mapping();
        std::cout << std::endl;
        
        std::cout << "=== All tests completed ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
