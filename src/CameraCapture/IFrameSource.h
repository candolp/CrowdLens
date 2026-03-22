#pragma once

#include <chrono>
#include <functional>
#include <opencv2/core.hpp>

namespace cl {

// called by the frame source for each new frame, passing the frame and when it was captured
using FrameCallback = std::function<void(cv::Mat, std::chrono::steady_clock::time_point)>;

class IFrameSource {
public:
    virtual ~IFrameSource() = default;

    virtual void setFrameCallback(FrameCallback callback) = 0;

    virtual void start() = 0;

    virtual void stop() = 0;
};

}
