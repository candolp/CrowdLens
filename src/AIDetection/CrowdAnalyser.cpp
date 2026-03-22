#include "CrowdAnalyser.h"

namespace cl {

CrowdAnalyser::CrowdAnalyser(std::unique_ptr<IFrameProcessor> processor, ZoneManager& manager)
    : zoneManager_(manager)
{
    processor_ = std::move(processor);
}

CrowdAnalyser::~CrowdAnalyser() {
    stop(traffic_state);
}


void CrowdAnalyser::setDisplayCallback(DisplayCallback callback) {
    displayCallback_ = std::move(callback);
}

void CrowdAnalyser::onFrameArrived(cv::Mat frame, std::chrono::steady_clock::time_point ts) {
    {
        std::lock_guard<std::mutex> lock(frameMutex_);
        pendingFrame_ = {std::move(frame), ts}; // drop-oldest: replace unconditionally
    }
    cv_.notify_one();
}

void CrowdAnalyser::run(TrafficState state) {
    running_.store(true);
    TrafficEventHandler::run(state);
}

void CrowdAnalyser::stop(TrafficState state) {
    running_.store(false);
    cv_.notify_all();
    TrafficEventHandler::stop(state);
}

void CrowdAnalyser::setDensityThreshold(float threshold) {
    if (threshold < 0.0f || threshold > 1.0f) return; // density is a ratio, must be between 0 and 1
    densityThreshold_ = threshold;
}

void CrowdAnalyser::setChokepointThreshold(float threshold) {
    if (threshold < 0.0f || threshold > 1.0f) return;
    chokepointThreshold_ = threshold;
}

void CrowdAnalyser::setFlowMagnitudeThreshold(float threshold) {
    if (threshold < 0.0f) return;
    flowMagnitudeThreshold_ = threshold;
}

void CrowdAnalyser::worker() {
    while (running_.load()) {
        // wait for a frame to arrive, then take it and release the lock
        std::unique_lock<std::mutex> lock(frameMutex_);
        cv_.wait(lock, [this]{ return pendingFrame_.has_value() || !running_.load(); });
        if (!running_.load()) break;
        auto [frame, ts] = std::move(*pendingFrame_);
        pendingFrame_.reset();
        lock.unlock();

        // run analysis on zones in frame
        std::vector<Zone> zones = zoneManager_.getZones();
        std::vector<CrowdMetrics> metrics = processor_->processFrame(frame, zones, ts);

        // check for congestion and chokepoint and alert
        for (CrowdMetrics& m : metrics) {
            if (m.density >= densityThreshold_) {
                AlertEvent ev{
                    TrafficState::CROWDED,
                    AlertType::CONGESTION,
                    AlertSeverity::WARNING,
                    m,
                    "Congestion detected"
                };
                alertCallback(ev);
                eventCallback(TrafficState::CROWDED);
            }
            if (m.density >= chokepointThreshold_ && m.flowMagnitude < flowMagnitudeThreshold_) {
                AlertEvent ev{
                    TrafficState::STAMPEDE,
                    AlertType::CHOKEPOINT,
                    AlertSeverity::CRITICAL,
                    m,
                    "Chokepoint detected"
                };
                alertCallback(ev);
                eventCallback(TrafficState::STAMPEDE);
            }
        }

        // send the frame + metrics + zones to the display thread if one is registered
        if (displayCallback_)
            displayCallback_(frame, metrics, zones);
    }
}

}
