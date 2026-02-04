#include "optistream/video_stream.hpp"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iostream>

namespace optistream {

class VideoStream::Impl {
public:
    Impl() : pipeline_(nullptr), appsink_(nullptr), 
             running_(false), frames_processed_(0), frames_dropped_(0),
             total_processing_time_(0.0) {}

    ~Impl() {
        stop();
    }

    bool start(const std::string& source, int width, int height, int fps) {
        if (running_) {
            std::cerr << "VideoStream already running" << std::endl;
            return false;
        }

        // Initialize GStreamer
        static bool gst_initialized = false;
        if (!gst_initialized) {
            gst_init(nullptr, nullptr);
            gst_initialized = true;
        }

        // Build pipeline string
        std::string pipeline_str = source + " ! "
            "videoconvert ! "
            "video/x-raw,format=BGR,width=" + std::to_string(width) + 
            ",height=" + std::to_string(height) + 
            ",framerate=" + std::to_string(fps) + "/1 ! "
            "appsink name=sink emit-signals=true max-buffers=1 drop=true";

        GError* error = nullptr;
        pipeline_ = gst_parse_launch(pipeline_str.c_str(), &error);
        
        if (error) {
            std::cerr << "Failed to create pipeline: " << error->message << std::endl;
            g_error_free(error);
            return false;
        }

        appsink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "sink");
        if (!appsink_) {
            std::cerr << "Failed to get appsink element" << std::endl;
            gst_object_unref(pipeline_);
            pipeline_ = nullptr;
            return false;
        }

        // Set appsink callbacks
        GstAppSinkCallbacks callbacks;
        callbacks.new_sample = &Impl::on_new_sample_static;
        callbacks.eos = nullptr;
        callbacks.new_preroll = nullptr;
        gst_app_sink_set_callbacks(GST_APP_SINK(appsink_), &callbacks, this, nullptr);

        // Start pipeline
        GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            std::cerr << "Failed to start pipeline" << std::endl;
            gst_object_unref(appsink_);
            gst_object_unref(pipeline_);
            appsink_ = nullptr;
            pipeline_ = nullptr;
            return false;
        }

        running_ = true;
        width_ = width;
        height_ = height;

        std::cout << "VideoStream started: " << width << "x" << height << " @ " << fps << " fps" << std::endl;
        return true;
    }

    void stop() {
        if (!running_) {
            return;
        }

        running_ = false;

        if (pipeline_) {
            gst_element_set_state(pipeline_, GST_STATE_NULL);
            gst_object_unref(pipeline_);
            pipeline_ = nullptr;
        }

        if (appsink_) {
            gst_object_unref(appsink_);
            appsink_ = nullptr;
        }

        std::cout << "VideoStream stopped" << std::endl;
    }

    void set_callback(FrameCallback callback) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        callback_ = callback;
    }

    bool is_running() const {
        return running_;
    }

    VideoStream::Stats get_stats() const {
        VideoStream::Stats stats;
        stats.frames_processed = frames_processed_.load();
        stats.frames_dropped = frames_dropped_.load();
        uint64_t processed = stats.frames_processed;
        stats.avg_processing_time_ms = processed > 0 ? 
            total_processing_time_.load() / processed : 0.0;
        return stats;
    }

private:
    static GstFlowReturn on_new_sample_static(GstAppSink* appsink, gpointer user_data) {
        Impl* self = static_cast<Impl*>(user_data);
        return self->on_new_sample(appsink);
    }

    GstFlowReturn on_new_sample(GstAppSink* appsink) {
        auto start_time = std::chrono::high_resolution_clock::now();

        // Pull sample from appsink
        GstSample* sample = gst_app_sink_pull_sample(appsink);
        if (!sample) {
            return GST_FLOW_ERROR;
        }

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstCaps* caps = gst_sample_get_caps(sample);

        if (!buffer || !caps) {
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        // Map buffer (zero-copy)
        GstMapInfo map;
        if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            gst_sample_unref(sample);
            frames_dropped_++;
            return GST_FLOW_ERROR;
        }

        // Create cv::Mat wrapper around buffer data (zero-copy)
        cv::Mat frame(height_, width_, CV_8UC3, map.data);

        // Get timestamp
        double timestamp = GST_BUFFER_PTS(buffer) / 1e9; // Convert to seconds

        // Call user callback
        {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            if (callback_) {
                callback_(frame, timestamp);
            }
        }

        // Cleanup
        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);

        frames_processed_++;

        auto end_time = std::chrono::high_resolution_clock::now();
        double processing_time = std::chrono::duration<double, std::milli>(
            end_time - start_time).count();
        total_processing_time_.fetch_add(processing_time);

        return GST_FLOW_OK;
    }

    GstElement* pipeline_;
    GstElement* appsink_;
    std::atomic<bool> running_;
    int width_;
    int height_;

    std::mutex callback_mutex_;
    FrameCallback callback_;

    std::atomic<uint64_t> frames_processed_;
    std::atomic<uint64_t> frames_dropped_;
    std::atomic<double> total_processing_time_;
};

// VideoStream implementation
VideoStream::VideoStream() : pimpl_(std::make_unique<Impl>()) {}

VideoStream::~VideoStream() = default;

bool VideoStream::start(const std::string& source, int width, int height, int fps) {
    return pimpl_->start(source, width, height, fps);
}

void VideoStream::stop() {
    pimpl_->stop();
}

void VideoStream::set_callback(FrameCallback callback) {
    pimpl_->set_callback(callback);
}

bool VideoStream::is_running() const {
    return pimpl_->is_running();
}

VideoStream::Stats VideoStream::get_stats() const {
    return pimpl_->get_stats();
}

} // namespace optistream
