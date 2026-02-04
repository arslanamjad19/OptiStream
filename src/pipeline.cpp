#include "optistream/pipeline.hpp"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <iostream>

namespace optistream {

class Pipeline::Impl {
public:
    Impl() : running_(false), max_queue_size_(5),
             frames_processed_(0), frames_dropped_(0),
             inferences_count_(0), total_frame_time_(0.0),
             total_inference_time_(0.0) {}

    ~Impl() {
        stop();
    }

    bool start(const std::string& video_source, 
               const std::string& model_path,
               int width, int height, int fps,
               size_t max_queue_size) {
        
        if (running_) {
            std::cerr << "Pipeline already running" << std::endl;
            return false;
        }

        max_queue_size_ = max_queue_size;

        // Load model
        if (!engine_.load_model(model_path)) {
            std::cerr << "Failed to load model" << std::endl;
            return false;
        }

        // Set up video stream callback
        stream_.set_callback([this](const cv::Mat& frame, double timestamp) {
            this->on_frame(frame, timestamp);
        });

        // Start video stream
        if (!stream_.start(video_source, width, height, fps)) {
            std::cerr << "Failed to start video stream" << std::endl;
            return false;
        }

        running_ = true;

        // Start inference thread
        inference_thread_ = std::thread(&Impl::inference_worker, this);

        std::cout << "Pipeline started" << std::endl;
        return true;
    }

    void stop() {
        if (!running_) {
            return;
        }

        running_ = false;

        // Stop video stream
        stream_.stop();

        // Wake up inference thread
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_cv_.notify_all();
        }

        // Wait for inference thread
        if (inference_thread_.joinable()) {
            inference_thread_.join();
        }

        // Clear queue
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            while (!frame_queue_.empty()) {
                frame_queue_.pop();
            }
        }

        std::cout << "Pipeline stopped" << std::endl;
    }

    void set_callback(InferenceCallback callback) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback_ = callback;
    }

    bool is_running() const {
        return running_;
    }

    Pipeline::Stats get_stats() const {
        Pipeline::Stats stats;
        
        auto stream_stats = stream_.get_stats();
        auto engine_stats = engine_.get_stats();
        
        stats.frames_processed = stream_stats.frames_processed;
        stats.frames_dropped = frames_dropped_.load();
        stats.inferences_count = engine_stats.inferences_count;
        stats.avg_frame_time_ms = stream_stats.avg_processing_time_ms;
        stats.avg_inference_time_ms = engine_stats.avg_inference_time_ms;
        
        return stats;
    }

private:
    struct FrameData {
        cv::Mat frame;
        double timestamp;
    };

    void on_frame(const cv::Mat& frame, double timestamp) {
        auto start_time = std::chrono::high_resolution_clock::now();

        // Clone frame for queue (necessary since GStreamer buffer will be released)
        cv::Mat cloned_frame = frame.clone();

        // Add to queue with frame drop policy
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            
            if (frame_queue_.size() >= max_queue_size_) {
                // Drop oldest frame
                frame_queue_.pop();
                frames_dropped_++;
            }
            
            frame_queue_.push({cloned_frame, timestamp});
            frames_processed_++;
        }
        
        queue_cv_.notify_one();

        auto end_time = std::chrono::high_resolution_clock::now();
        double frame_time = std::chrono::duration<double, std::milli>(
            end_time - start_time).count();
        total_frame_time_ += frame_time;
    }

    void inference_worker() {
        std::cout << "Inference worker thread started" << std::endl;

        while (running_) {
            FrameData data;
            
            // Get frame from queue
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait(lock, [this]() {
                    return !frame_queue_.empty() || !running_;
                });

                if (!running_ && frame_queue_.empty()) {
                    break;
                }

                if (!frame_queue_.empty()) {
                    data = frame_queue_.front();
                    frame_queue_.pop();
                }
            }

            if (data.frame.empty()) {
                continue;
            }

            // Run inference
            auto start_time = std::chrono::high_resolution_clock::now();
            std::vector<float> output = engine_.infer(data.frame);
            auto end_time = std::chrono::high_resolution_clock::now();

            double inference_time = std::chrono::duration<double, std::milli>(
                end_time - start_time).count();
            total_inference_time_ += inference_time;
            inferences_count_++;

            // Call user callback
            {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                if (callback_ && !output.empty()) {
                    callback_(output, data.timestamp);
                }
            }
        }

        std::cout << "Inference worker thread stopped" << std::endl;
    }

    VideoStream stream_;
    InferenceEngine engine_;

    std::atomic<bool> running_;
    size_t max_queue_size_;

    std::queue<FrameData> frame_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    std::thread inference_thread_;

    std::mutex callback_mutex_;
    InferenceCallback callback_;

    std::atomic<uint64_t> frames_processed_;
    std::atomic<uint64_t> frames_dropped_;
    std::atomic<uint64_t> inferences_count_;
    std::atomic<double> total_frame_time_;
    std::atomic<double> total_inference_time_;
};

// Pipeline implementation
Pipeline::Pipeline() : pimpl_(std::make_unique<Impl>()) {}

Pipeline::~Pipeline() = default;

bool Pipeline::start(const std::string& video_source, 
                     const std::string& model_path,
                     int width, int height, int fps,
                     size_t max_queue_size) {
    return pimpl_->start(video_source, model_path, width, height, fps, max_queue_size);
}

void Pipeline::stop() {
    pimpl_->stop();
}

void Pipeline::set_callback(InferenceCallback callback) {
    pimpl_->set_callback(callback);
}

bool Pipeline::is_running() const {
    return pimpl_->is_running();
}

Pipeline::Stats Pipeline::get_stats() const {
    return pimpl_->get_stats();
}

} // namespace optistream
