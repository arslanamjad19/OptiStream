#include "optistream/inference_engine.hpp"
#include <onnxruntime/core/session/onnxruntime_cxx_api.h>
#include <chrono>
#include <iostream>
#include <algorithm>

namespace optistream {

class InferenceEngine::Impl {
public:
    Impl() : env_(ORT_LOGGING_LEVEL_WARNING, "OptiStream"),
             session_(nullptr), loaded_(false),
             inferences_count_(0), total_inference_time_(0.0) {}

    ~Impl() = default;

    bool load_model(const std::string& model_path) {
        try {
            Ort::SessionOptions session_options;
            session_options.SetIntraOpNumThreads(1);
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

            // Create session
#ifdef _WIN32
            std::wstring wmodel_path(model_path.begin(), model_path.end());
            session_ = std::make_unique<Ort::Session>(env_, wmodel_path.c_str(), session_options);
#else
            session_ = std::make_unique<Ort::Session>(env_, model_path.c_str(), session_options);
#endif

            // Get input shape
            Ort::AllocatorWithDefaultOptions allocator;
            
            size_t num_input_nodes = session_->GetInputCount();
            if (num_input_nodes > 0) {
                input_name_ = session_->GetInputNameAllocated(0, allocator).get();
                
                Ort::TypeInfo type_info = session_->GetInputTypeInfo(0);
                auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
                input_shape_ = tensor_info.GetShape();
                
                std::cout << "Model loaded: " << model_path << std::endl;
                std::cout << "Input shape: [";
                for (size_t i = 0; i < input_shape_.size(); i++) {
                    std::cout << input_shape_[i];
                    if (i < input_shape_.size() - 1) std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            }

            size_t num_output_nodes = session_->GetOutputCount();
            if (num_output_nodes > 0) {
                output_name_ = session_->GetOutputNameAllocated(0, allocator).get();
            }

            loaded_ = true;
            return true;
        } catch (const Ort::Exception& e) {
            std::cerr << "ONNX Runtime error: " << e.what() << std::endl;
            loaded_ = false;
            return false;
        }
    }

    std::vector<float> infer(const cv::Mat& frame) {
        if (!loaded_ || !session_) {
            return {};
        }

        auto start_time = std::chrono::high_resolution_clock::now();

        try {
            // Prepare input tensor
            // Assume input shape is [1, 3, H, W] or [1, H, W, 3]
            int64_t batch_size = 1;
            int64_t channels = 3;
            int64_t height = input_shape_.size() > 2 ? input_shape_[2] : 640;
            int64_t width = input_shape_.size() > 3 ? input_shape_[3] : 640;
            
            // Resize frame if needed
            cv::Mat resized_frame;
            if (frame.rows != height || frame.cols != width) {
                cv::resize(frame, resized_frame, cv::Size(width, height));
            } else {
                resized_frame = frame;
            }

            // Convert to float and normalize
            cv::Mat float_frame;
            resized_frame.convertTo(float_frame, CV_32FC3, 1.0 / 255.0);

            // Prepare input data (NCHW format)
            std::vector<float> input_data(batch_size * channels * height * width);
            
            // Convert BGR to RGB and transpose to NCHW
            for (int h = 0; h < height; h++) {
                for (int w = 0; w < width; w++) {
                    cv::Vec3f pixel = float_frame.at<cv::Vec3f>(h, w);
                    // BGR to RGB
                    input_data[0 * height * width + h * width + w] = pixel[2]; // R
                    input_data[1 * height * width + h * width + w] = pixel[1]; // G
                    input_data[2 * height * width + h * width + w] = pixel[0]; // B
                }
            }

            // Create input tensor
            std::vector<int64_t> input_shape = {batch_size, channels, height, width};
            auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                memory_info, input_data.data(), input_data.size(),
                input_shape.data(), input_shape.size());

            // Run inference
            const char* input_names[] = {input_name_.c_str()};
            const char* output_names[] = {output_name_.c_str()};
            
            auto output_tensors = session_->Run(
                Ort::RunOptions{nullptr},
                input_names, &input_tensor, 1,
                output_names, 1);

            // Get output
            float* output_data = output_tensors[0].GetTensorMutableData<float>();
            auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
            
            size_t output_size = 1;
            for (auto dim : output_shape) {
                output_size *= dim;
            }

            std::vector<float> output(output_data, output_data + output_size);

            inferences_count_++;

            auto end_time = std::chrono::high_resolution_clock::now();
            double inference_time = std::chrono::duration<double, std::milli>(
                end_time - start_time).count();
            total_inference_time_ += inference_time;

            return output;
        } catch (const Ort::Exception& e) {
            std::cerr << "Inference error: " << e.what() << std::endl;
            return {};
        }
    }

    bool is_loaded() const {
        return loaded_;
    }

    std::vector<int64_t> get_input_shape() const {
        return input_shape_;
    }

    InferenceEngine::Stats get_stats() const {
        InferenceEngine::Stats stats;
        stats.inferences_count = inferences_count_;
        stats.avg_inference_time_ms = inferences_count_ > 0 ?
            total_inference_time_ / inferences_count_ : 0.0;
        return stats;
    }

private:
    Ort::Env env_;
    std::unique_ptr<Ort::Session> session_;
    bool loaded_;
    
    std::string input_name_;
    std::string output_name_;
    std::vector<int64_t> input_shape_;

    uint64_t inferences_count_;
    double total_inference_time_;
};

// InferenceEngine implementation
InferenceEngine::InferenceEngine() : pimpl_(std::make_unique<Impl>()) {}

InferenceEngine::~InferenceEngine() = default;

bool InferenceEngine::load_model(const std::string& model_path) {
    return pimpl_->load_model(model_path);
}

std::vector<float> InferenceEngine::infer(const cv::Mat& frame) {
    return pimpl_->infer(frame);
}

bool InferenceEngine::is_loaded() const {
    return pimpl_->is_loaded();
}

std::vector<int64_t> InferenceEngine::get_input_shape() const {
    return pimpl_->get_input_shape();
}

InferenceEngine::Stats InferenceEngine::get_stats() const {
    return pimpl_->get_stats();
}

} // namespace optistream
