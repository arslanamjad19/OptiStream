#include <optistream/pipeline.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

// Global flag for clean shutdown
volatile sig_atomic_t keep_running = 1;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nReceived interrupt signal..." << std::endl;
        keep_running = 0;
    }
}

int main(int argc, char** argv) {
    // Set up signal handler for clean shutdown
    signal(SIGINT, signal_handler);
    
    std::cout << "OptiStream Simple Pipeline Example" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << std::endl;
    
    // Parse command line arguments
    std::string video_source = "videotestsrc pattern=0";
    std::string model_path = "models/yolov5s.onnx";
    
    if (argc > 1) {
        video_source = argv[1];
    }
    if (argc > 2) {
        model_path = argv[2];
    }
    
    std::cout << "Video source: " << video_source << std::endl;
    std::cout << "Model path: " << model_path << std::endl;
    std::cout << std::endl;
    
    // Create pipeline
    optistream::Pipeline pipeline;
    
    // Set up callback for inference results
    pipeline.set_callback([](const std::vector<float>& output, double timestamp) {
        static int count = 0;
        count++;
        
        if (count == 1 || count % 30 == 0) {
            std::cout << "[" << timestamp << "s] Inference #" << count 
                      << " - Output size: " << output.size() << std::endl;
        }
    });
    
    // Start the pipeline
    std::cout << "Starting pipeline..." << std::endl;
    if (!pipeline.start(
        video_source,
        model_path,
        640, 640,  // width, height
        30,        // fps
        5          // max queue size
    )) {
        std::cerr << "Failed to start pipeline" << std::endl;
        return 1;
    }
    
    std::cout << "Pipeline started successfully!" << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;
    std::cout << std::endl;
    
    // Run until interrupted
    while (keep_running && pipeline.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop pipeline
    std::cout << std::endl;
    std::cout << "Stopping pipeline..." << std::endl;
    pipeline.stop();
    
    // Print statistics
    auto stats = pipeline.get_stats();
    std::cout << std::endl;
    std::cout << "Pipeline Statistics:" << std::endl;
    std::cout << "  Frames processed: " << stats.frames_processed << std::endl;
    std::cout << "  Frames dropped: " << stats.frames_dropped << std::endl;
    std::cout << "  Inferences: " << stats.inferences_count << std::endl;
    std::cout << "  Avg frame time: " << stats.avg_frame_time_ms << " ms" << std::endl;
    std::cout << "  Avg inference time: " << stats.avg_inference_time_ms << " ms" << std::endl;
    
    if (stats.frames_processed > 0) {
        double drop_rate = (double)stats.frames_dropped / stats.frames_processed * 100.0;
        std::cout << "  Frame drop rate: " << drop_rate << "%" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Done!" << std::endl;
    
    return 0;
}
