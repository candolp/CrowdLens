#include "CrowdAnalyser.h"
#include "../Common/ConfigLoader.h"

namespace cl {

CrowdAnalyser::CrowdAnalyser(std::unique_ptr<IFrameProcessor> processor, ZoneManager& manager)
    : zoneManager_(manager)
{
    processor_ = std::move(processor);
}

CrowdAnalyser::CrowdAnalyser(std::unique_ptr<IFrameProcessor> processor, ZoneManager& manager, const ConfigLoader& config)
    : CrowdAnalyser(std::move(processor), manager)
{
    loadConfig(config);
}

void CrowdAnalyser::loadConfig(const ConfigLoader& config) {
    setDensityThreshold(std::stof(config.getValue("thresholds:density", "0.7")));
    setChokepointThreshold(std::stof(config.getValue("thresholds:chokepoint", "0.85")));
    setFlowMagnitudeThreshold(std::stof(config.getValue("thresholds:flow_magnitude", "2.0")));
    setStampedeDensityThreshold(std::stof(config.getValue("prediction:stampede_density", "0.9")));
    setPredictionHorizon(std::stof(config.getValue("prediction:horizon", "10.0")));
    setPredictionWindowSize(std::stoul(config.getValue("prediction:window_size", "10")));
    setMinTrendSlope(std::stof(config.getValue("prediction:min_trend_slope", "0.005")));
    setWarmupFrames(std::stoi(config.getValue("thresholds:warmup_frames", "30")));
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
    frameCount_ = 0;
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
    predictor_.setFlowMagnitudeThreshold(threshold);
}

void CrowdAnalyser::setPredictionHorizon(float seconds) {
    predictor_.setPredictionHorizon(seconds);
}

void CrowdAnalyser::setStampedeDensityThreshold(float t) {
    predictor_.setStampedeDensityThreshold(t);
}

void CrowdAnalyser::setPredictionWindowSize(size_t n) {
    predictor_.setWindowSize(n);
}

void CrowdAnalyser::setMinTrendSlope(float slope) {
    predictor_.setMinTrendSlope(slope);
}

void CrowdAnalyser::setWarmupFrames(int frames) {
    if (frames < 0) return;
    warmupFrames_ = frames;
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

        frameCount_++;

        // skip alert evaluation until the bg model has had enough frames to stabilise
        if (frameCount_ > warmupFrames_) {
            auto now = std::chrono::steady_clock::now();
            bool anyAlert = false;

            for (CrowdMetrics& m : metrics) {
                if (m.density >= densityThreshold_) {
                    anyAlert = true;
                    if (now - lastAlertTime_[AlertType::CONGESTION] >= alertCooldown) {
                        lastAlertTime_[AlertType::CONGESTION] = now;
                        AlertEvent ev{
                            TrafficState::CROWDED,
                            AlertType::CONGESTION,
                            AlertSeverity::WARNING,
                            m,
                            "Congestion detected"
                        };
                        alertCallback(ev);
                        CrowdAnalyser::eventCallback(TrafficState::CROWDED);
                    }
                }
                if (m.density >= chokepointThreshold_ && m.flowMagnitude < flowMagnitudeThreshold_) {
                    anyAlert = true;
                    if (now - lastAlertTime_[AlertType::CHOKEPOINT] >= alertCooldown) {
                        lastAlertTime_[AlertType::CHOKEPOINT] = now;
                        AlertEvent ev{
                            TrafficState::STAMPEDE,
                            AlertType::CHOKEPOINT,
                            AlertSeverity::CRITICAL,
                            m,
                            "Chokepoint detected"
                        };
                        alertCallback(ev);
                        CrowdAnalyser::eventCallback(TrafficState::STAMPEDE);
                    }
                }

                // run predictor and fire early-warning alerts if trends indicate we're heading toward critical levels
                predictor_.update(m);
                StampedePredictor::Prediction pred = predictor_.predict(m.zoneName);
                if (pred.stampedeRisk) {
                    anyAlert = true;
                    if (now - lastAlertTime_[AlertType::STAMPEDE_RISK] >= alertCooldown) {
                        lastAlertTime_[AlertType::STAMPEDE_RISK] = now;
                        std::string msg = "Stampede risk: density rising fast, ~"
                            + std::to_string(static_cast<int>(pred.timeToStampede))
                            + "s to critical level";
                        AlertEvent ev{ TrafficState::STAMPEDE, AlertType::STAMPEDE_RISK, AlertSeverity::WARNING, m, msg };
                        alertCallback(ev);
                    }
                }
                if (pred.chokepointRisk) {
                    anyAlert = true;
                    if (now - lastAlertTime_[AlertType::CHOKEPOINT_PREDICTED] >= alertCooldown) {
                        lastAlertTime_[AlertType::CHOKEPOINT_PREDICTED] = now;
                        std::string msg = "Chokepoint predicted: density rising, flow falling, ~"
                            + std::to_string(static_cast<int>(pred.timeToChokepoint))
                            + "s to chokepoint";
                        AlertEvent ev{ TrafficState::CROWDED, AlertType::CHOKEPOINT_PREDICTED, AlertSeverity::WARNING, m, msg };
                        alertCallback(ev);
                    }
                }
            }

            // nothing triggered, return to default state
            if (!anyAlert) {
                if (now - lastAlertTime_[AlertType::NO_ALERT] >= alertCooldown) {
                    lastAlertTime_[AlertType::NO_ALERT] = now;
                    AlertEvent ev{ TrafficState::NO_TRAFFIC, AlertType::NO_ALERT, AlertSeverity::INFO, {}, "Normal" };
                    alertCallback(ev);
                    CrowdAnalyser::eventCallback(TrafficState::NO_TRAFFIC);
                }
            }
        }

        // send the frame + metrics + zones to the display thread if one is registered
        if (displayCallback_)
            displayCallback_(frame, metrics, zones);
    }
}



    void CrowdAnalyser::eventCallback(TrafficState trafficState) {
    for(auto & r : eventHandlers) {
        r->run(trafficState);
    }
}

}
