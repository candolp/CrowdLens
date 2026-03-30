#pragma once

#include "IFrameSource.h"

#include <atomic>
#include <thread>
#include <opencv2/videoio.hpp>

namespace cl {

// reads frames from a camera using cv::VideoCapture and fires the callback on each one
class CameraFrameSource : public IFrameSource {
public:
    // 0 = default camera
    explicit CameraFrameSource(int deviceIndex = 0);
    ~CameraFrameSource() override;

    void setFrameCallback(FrameCallback callback) override;
    void start() override;
    void stop() override;

private:
    // runs on the capture thread: loops calling read(), fires callback_ on each valid frame
    void captureLoop();

    int deviceIndex_;
    FrameCallback callback_;
    cv::VideoCapture capture_;
    std::thread captureThread_;
    std::atomic<bool> running_{false};
};

}
