#pragma once

#include "IFrameSource.h"

#include <atomic>
#include <string>
#include <thread>
#include <opencv2/videoio.hpp>

namespace cl {

// reads frames from a video file and fires the callback at the file's native FPS
class VideoFileFrameSource : public IFrameSource {
public:
    explicit VideoFileFrameSource(const std::string& filePath);
    ~VideoFileFrameSource() override;

    void setFrameCallback(FrameCallback callback) override;
    void start() override;
    void stop() override;

private:
    // reads frames in a loop, pacing them to match the file's FPS, exits at EOF
    void captureLoop();

    std::string filePath_;
    FrameCallback callback_;
    cv::VideoCapture capture_;
    std::thread captureThread_;
    std::atomic<bool> running_{false};
};

}
