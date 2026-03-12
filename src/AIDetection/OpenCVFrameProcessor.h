#pragma once

#include "IFrameProcessor.h"

#include <memory>
#include <opencv2/video/background_segm.hpp>

namespace cl {

// uses background subtraction to spot people and optical flow to track movement
// bgSubtractor_ extracts the foreground mask; prevGrey_ holds the last frame so we can compute flow
class OpenCVFrameProcessor : public IFrameProcessor {
public:
    OpenCVFrameProcessor();
    ~OpenCVFrameProcessor() override = default;

    // processes one frame: converts to greyscale, runs bg subtraction, computes optical flow if we have a previous frame
    // returns one CrowdMetrics per zone with density + flow direction filled in
    std::vector<CrowdMetrics> processFrame(
        const cv::Mat& frame,
        const std::vector<Zone>& zones,
        std::chrono::steady_clock::time_point ts) override;

private:
    cv::Ptr<cv::BackgroundSubtractorMOG2> bgSubtractor_;
    cv::Mat prevGrey_; // greyscale copy of the last frame, needed for optical flow
};

}
