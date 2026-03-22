#pragma once

#include "../Common/CrowdMetrics.h"
#include "Zone.h"
#include "../Common/CrowdMetrics.h"

#include <chrono>
#include <vector>
#include <opencv2/core.hpp>

namespace cl {

class IFrameProcessor {
public:
    virtual ~IFrameProcessor() = default;

    // takes a frame + the zone list and returns metrics for each zone
    virtual std::vector<CrowdMetrics> processFrame(
        const cv::Mat& frame,
        const std::vector<Zone>& zones,
        std::chrono::steady_clock::time_point ts) = 0;
};

}
