#include "CameraFrameSource.h"

#include <iostream>

namespace cl {

CameraFrameSource::CameraFrameSource(int deviceIndex) {
    deviceIndex_ = deviceIndex;
}

CameraFrameSource::~CameraFrameSource() {
    stop();
}

void CameraFrameSource::setFrameCallback(FrameCallback callback) {
    callback_ = std::move(callback);
}

void CameraFrameSource::start() {
    if (running_.exchange(true)) return; // already running
    captureThread_ = std::thread(&CameraFrameSource::captureLoop, this);
}

void CameraFrameSource::stop() {
    running_.store(false);
    if (captureThread_.joinable()) {
        captureThread_.join();
    }
}

void CameraFrameSource::captureLoop() {
    capture_.open(deviceIndex_);
    if (!capture_.isOpened())
    {
        return;
    }

    cv::Mat frame;
    while (running_.load())
    {
        bool ok = capture_.read(frame);
        if (!ok)
        {
            std::cerr << "Capture returned false" << std::endl;
            break;
        }

        // timestamp after read() so it reflects when the frame actually arrived
        std::chrono::steady_clock::time_point ts = std::chrono::steady_clock::now();
        if (callback_)
        {
            callback_(frame, ts);
        }
}
    capture_.release();
}

}
