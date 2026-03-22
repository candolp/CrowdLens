#pragma once

#include "../Common/AlertEvent.h"
#include "../Common/AlertRunnable.h"
#include "IFrameProcessor.h"
#include "ZoneManager.h"
#include "../Common/TrafficEventHandler.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>
#include <opencv2/core.hpp>

namespace cl {

// fired each analysis cycle so the display can get the latest frame + metrics
using DisplayCallback = std::function<void(cv::Mat, std::vector<CrowdMetrics>, std::vector<Zone>)>;

// gets frames, runs analysis, fires alerts to registered subscribers
class CrowdAnalyser : public TrafficEventHandler {
public:
    // processor does the actual OpenCV work, manager holds the zones
    CrowdAnalyser(std::unique_ptr<IFrameProcessor> processor, ZoneManager& manager);
    ~CrowdAnalyser();

    void setDisplayCallback(DisplayCallback callback);

    // camera thread calls this, drop the old frame and wake the analysis thread
    void onFrameArrived(cv::Mat frame, std::chrono::steady_clock::time_point ts);

    // sets running_ then starts the worker thread via TrafficEventHandler::run()
    void run(TrafficState state) override;

    // overrides TrafficEventHandler::stop() to wake the CV before joining
    void stop(TrafficState state) override;

    // fires a congestion alert if threshold passed
    void setDensityThreshold(float threshold);

    // fires a chokepoint alert if occupancy is high and flow is low
    void setChokepointThreshold(float threshold);

    // low-flow cutoff used alongside chokepointThreshold to detect chokepoints
    void setFlowMagnitudeThreshold(float threshold);

private:
    // waits for frames and processes them; implements TrafficEventHandler::worker()
    void worker() override;

    std::unique_ptr<IFrameProcessor> processor_;
    ZoneManager& zoneManager_;
    DisplayCallback displayCallback_;

    using FrameEntry = std::pair<cv::Mat, std::chrono::steady_clock::time_point>;
    std::optional<FrameEntry> pendingFrame_;
    std::mutex frameMutex_;
    std::condition_variable cv_;

    std::atomic<bool> running_ = false;

    // TODO: tune these based on camera height and resolution after integration
    float densityThreshold_ = 0.7f; // 70% zone occupancy -> CONGESTION
    float chokepointThreshold_ = 0.85f; // 85% occupancy + low flow -> CHOKEPOINT
    float flowMagnitudeThreshold_ = 2.0f; // low-flow cutoff for chokepoint
};

}
