#pragma once

#include "../Common/AlertRunnable.h"
#include "../Common/CrowdMetrics.h"
#include "../AIDetection/Zone.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
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

    // AlertRunnable::run() -- overlay is driven by its own thread, not by event state
    void run(TrafficState /*state*/) override {}

    // Runnable::stop -- no-op, FrameOverlay lifecycle is controlled by stop() below
    void stop(TrafficState /*state*/) override {}

    // lifecycle stop: signals renderWorker to exit and joins the thread
    void stop();

    void start();

    // blocks until the display thread exits ('q' pressed or stop() called)
    void runUntilClosed();

private:
    // draws zone polygons, labels, flow arrows, and alert banner onto frame
    void drawOverlay(cv::Mat& frame, const std::vector<CrowdMetrics>& metrics, const std::vector<Zone>& zones);

    void renderWorker();  // display thread body

    std::thread displayThread_;

    std::string windowName_;

    struct DisplayEntry {
        cv::Mat frame;
        std::vector<CrowdMetrics> metrics;
        std::vector<Zone> zones;
    };
    std::optional<DisplayEntry> pending_;
    std::mutex frameMutex_;
    std::condition_variable frameCv_;

    std::optional<AlertEvent> latestAlert_;
    std::chrono::steady_clock::time_point alertReceivedAt_;
    std::mutex alertMutex_;

    std::atomic<bool> running_{false};
    std::atomic<bool> windowOpen_{false};

    // how long the alert banner stays visible after last onAlert call
    static constexpr std::chrono::seconds kBannerDuration{2};
};

}
