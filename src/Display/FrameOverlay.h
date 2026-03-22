#pragma once

#include "../Common/AlertRunnable.h"
#include "../Common/CrowdMetrics.h"
#include "../AIDetection/Zone.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <opencv2/core.hpp>

namespace cl {

class FrameOverlay : public AlertRunnable {
public:
    explicit FrameOverlay(const std::string& windowName);
    ~FrameOverlay();

    // deposits frame into display queue
    void pushFrame(cv::Mat frame, std::vector<CrowdMetrics> metrics, std::vector<Zone> zones);

    // stores latest alert for the banner
    void onAlert(const AlertEvent& event) override;

    // AlertRunnable::run() -- overlay is driven by tick(), not by event state
    void run(TrafficState /*state*/) override {}

    // Runnable::stop -- no-op, FrameOverlay lifecycle is controlled by stop() below
    void stop(TrafficState /*state*/) override {}

    // lifecycle stop: sets running_ false so tick() returns false
    void stop();

    void start();

    // call from the main thread once per loop iteration; returns false when 'q' is pressed or stop() has been called
    bool tick();

private:
    // draws zone polygons, labels, flow arrows, and alert banner onto frame
    void drawOverlay(cv::Mat& frame, const std::vector<CrowdMetrics>& metrics, const std::vector<Zone>& zones);

    std::string windowName_;

    struct DisplayEntry {
        cv::Mat frame;
        std::vector<CrowdMetrics> metrics;
        std::vector<Zone> zones;
    };
    std::optional<DisplayEntry> pending_;
    std::mutex frameMutex_;

    std::optional<AlertEvent> latestAlert_;
    std::chrono::steady_clock::time_point alertReceivedAt_;
    std::mutex alertMutex_;

    std::atomic<bool> running_{false};
    std::atomic<bool> windowOpen_{false};

    // how long the alert banner stays visible after last onAlert call
    static constexpr std::chrono::seconds kBannerDuration{2};
};

}
