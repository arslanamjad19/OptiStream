#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "optistream/video_stream.hpp"
#include "optistream/inference_engine.hpp"
#include "optistream/pipeline.hpp"

namespace py = pybind11;

// Convert cv::Mat to numpy array
py::array_t<uint8_t> mat_to_numpy(const cv::Mat& mat) {
    py::array_t<uint8_t> arr({mat.rows, mat.cols, mat.channels()});
    auto buf = arr.request();
    uint8_t* ptr = static_cast<uint8_t*>(buf.ptr);
    std::memcpy(ptr, mat.data, mat.total() * mat.elemSize());
    return arr;
}

PYBIND11_MODULE(optistream, m) {
    m.doc() = "OptiStream: High-performance video processing and inference SDK";

    // VideoStream class
    py::class_<optistream::VideoStream>(m, "VideoStream")
        .def(py::init<>())
        .def("start", &optistream::VideoStream::start,
             py::arg("source"),
             py::arg("width") = 640,
             py::arg("height") = 480,
             py::arg("fps") = 30,
             "Start the video stream")
        .def("stop", &optistream::VideoStream::stop, "Stop the video stream")
        .def("set_callback", [](optistream::VideoStream& self, py::function callback) {
            // Wrap Python callback with GIL handling
            self.set_callback([callback](const cv::Mat& frame, double timestamp) {
                py::gil_scoped_acquire acquire;
                try {
                    auto np_frame = mat_to_numpy(frame);
                    callback(np_frame, timestamp);
                } catch (const py::error_already_set& e) {
                    std::cerr << "Python callback error: " << e.what() << std::endl;
                }
            });
        }, py::arg("callback"), "Set frame callback function")
        .def("is_running", &optistream::VideoStream::is_running, "Check if stream is running")
        .def("get_stats", &optistream::VideoStream::get_stats, "Get stream statistics");

    // VideoStream::Stats
    py::class_<optistream::VideoStream::Stats>(m, "VideoStreamStats")
        .def_readonly("frames_processed", &optistream::VideoStream::Stats::frames_processed)
        .def_readonly("frames_dropped", &optistream::VideoStream::Stats::frames_dropped)
        .def_readonly("avg_processing_time_ms", &optistream::VideoStream::Stats::avg_processing_time_ms);

    // InferenceEngine class
    py::class_<optistream::InferenceEngine>(m, "InferenceEngine")
        .def(py::init<>())
        .def("load_model", &optistream::InferenceEngine::load_model,
             py::arg("model_path"),
             "Load ONNX model")
        .def("infer", [](optistream::InferenceEngine& self, py::array_t<uint8_t> frame) {
            py::gil_scoped_release release;
            auto buf = frame.request();
            if (buf.ndim != 3) {
                throw std::runtime_error("Frame must be 3-dimensional");
            }
            cv::Mat mat(buf.shape[0], buf.shape[1], CV_8UC3, buf.ptr);
            return self.infer(mat);
        }, py::arg("frame"), "Run inference on frame")
        .def("is_loaded", &optistream::InferenceEngine::is_loaded, "Check if model is loaded")
        .def("get_input_shape", &optistream::InferenceEngine::get_input_shape, "Get input shape")
        .def("get_stats", &optistream::InferenceEngine::get_stats, "Get inference statistics");

    // InferenceEngine::Stats
    py::class_<optistream::InferenceEngine::Stats>(m, "InferenceEngineStats")
        .def_readonly("inferences_count", &optistream::InferenceEngine::Stats::inferences_count)
        .def_readonly("avg_inference_time_ms", &optistream::InferenceEngine::Stats::avg_inference_time_ms);

    // Pipeline class
    py::class_<optistream::Pipeline>(m, "Pipeline")
        .def(py::init<>())
        .def("start", &optistream::Pipeline::start,
             py::arg("video_source"),
             py::arg("model_path"),
             py::arg("width") = 640,
             py::arg("height") = 480,
             py::arg("fps") = 30,
             py::arg("max_queue_size") = 5,
             "Start the pipeline")
        .def("stop", &optistream::Pipeline::stop, "Stop the pipeline")
        .def("set_callback", [](optistream::Pipeline& self, py::function callback) {
            // Wrap Python callback with GIL handling
            self.set_callback([callback](const std::vector<float>& output, double timestamp) {
                py::gil_scoped_acquire acquire;
                try {
                    callback(output, timestamp);
                } catch (const py::error_already_set& e) {
                    std::cerr << "Python callback error: " << e.what() << std::endl;
                }
            });
        }, py::arg("callback"), "Set inference callback function")
        .def("is_running", &optistream::Pipeline::is_running, "Check if pipeline is running")
        .def("get_stats", &optistream::Pipeline::get_stats, "Get pipeline statistics");

    // Pipeline::Stats
    py::class_<optistream::Pipeline::Stats>(m, "PipelineStats")
        .def_readonly("frames_processed", &optistream::Pipeline::Stats::frames_processed)
        .def_readonly("frames_dropped", &optistream::Pipeline::Stats::frames_dropped)
        .def_readonly("inferences_count", &optistream::Pipeline::Stats::inferences_count)
        .def_readonly("avg_frame_time_ms", &optistream::Pipeline::Stats::avg_frame_time_ms)
        .def_readonly("avg_inference_time_ms", &optistream::Pipeline::Stats::avg_inference_time_ms);
}
