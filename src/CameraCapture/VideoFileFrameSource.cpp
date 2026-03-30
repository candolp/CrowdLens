#include "VideoFileFrameSource.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace cl {

VideoFileFrameSource::VideoFileFrameSource(const std::string& filePath) {
    filePath_ = filePath;
}

VideoFileFrameSource::~VideoFileFrameSource() {
    stop();
}

void VideoFileFrameSource::setFrameCallback(FrameCallback callback) {
    callback_ = std::move(callback);
}

void VideoFileFrameSource::start() {
    if (running_.exchange(true)) return;
    captureThread_ = std::thread(&VideoFileFrameSource::captureLoop, this);
}

void VideoFileFrameSource::stop() {
    running_.store(false);
    if (captureThread_.joinable()) {
        captureThread_.join();
    }
}

void VideoFileFrameSource::captureLoop() {
    capture_.open(filePath_);
    if (!capture_.isOpened()) {
        std::cerr << "VideoFileFrameSource: could not open file: " << filePath_ << std::endl;
        return;
    }

    // read the file's FPS so we can pace playback correctly
    double fps = capture_.get(cv::CAP_PROP_FPS);
    if (fps <= 0.0) {
        fps = 30.0;
    }

    std::chrono::nanoseconds frameDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(1.0 / fps)
    );

    cv::Mat frame;
    std::chrono::steady_clock::time_point nextFrameTime = std::chrono::steady_clock::now();

    while (running_.load()) {
        bool ok = capture_.read(frame);
        // loop the video for local testing
        if (!ok) {
            capture_.set(cv::CAP_PROP_POS_FRAMES, 0);
            nextFrameTime = std::chrono::steady_clock::now();
            continue;
        }

        std::chrono::steady_clock::time_point ts = std::chrono::steady_clock::now();
        if (callback_) {
            callback_(frame, ts);
        }

        // pace the loop so we dont read faster than the file's FPS
        nextFrameTime += frameDuration;
        std::this_thread::sleep_until(nextFrameTime);
    }

    capture_.release();
    running_.store(false);
}

}
