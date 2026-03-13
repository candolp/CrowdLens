#include "FrameOverlay.h"

#include <cmath>
#include <cstdio>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace cl {

FrameOverlay::FrameOverlay(const std::string& windowName) {
    windowName_ = windowName;
}

FrameOverlay::~FrameOverlay() {
    stop();
}

void FrameOverlay::pushFrame(cv::Mat frame, std::vector<CrowdMetrics> metrics, std::vector<Zone> zones) {
    {
        std::lock_guard<std::mutex> lock(frameMutex_);
        // drop-oldest: replace whatever is pending
        pending_ = DisplayEntry{ std::move(frame), std::move(metrics), std::move(zones) };
    }
    frameCv_.notify_one();
}

void FrameOverlay::onAlert(const AlertEvent& event) {
    std::lock_guard<std::mutex> lock(alertMutex_);
    latestAlert_ = event;
    alertReceivedAt_ = std::chrono::steady_clock::now();
}

void FrameOverlay::start() {
    if (running_) return;
    running_ = true;
    displayThread_ = std::thread(&FrameOverlay::renderWorker, this);
}

void FrameOverlay::stop() {
    running_.store(false);
    frameCv_.notify_all();  // wake renderWorker so it sees running_=false
    if (displayThread_.joinable())
        displayThread_.join();
    if (windowOpen_.exchange(false))
        cv::destroyWindow(windowName_);
}

void FrameOverlay::runUntilClosed() {
    if (displayThread_.joinable())
        displayThread_.join();
}

void FrameOverlay::renderWorker() {
    while (running_) {
        std::optional<DisplayEntry> entry;
        {
            std::unique_lock<std::mutex> lock(frameMutex_);
            frameCv_.wait(lock, [this] { return pending_.has_value() || !running_; });
            if (!running_) break;
            entry = std::move(pending_);
            pending_.reset();
        }

        drawOverlay(entry->frame, entry->metrics, entry->zones);
        cv::imshow(windowName_, entry->frame);
        windowOpen_.store(true);

        if (cv::waitKey(1) == 'q')
            running_.store(false);
    }

    if (windowOpen_.exchange(false))
        cv::destroyWindow(windowName_);
}

void FrameOverlay::drawOverlay(cv::Mat& frame, const std::vector<CrowdMetrics>& metrics,
    const std::vector<Zone>& zones) {

    for (const Zone& zone : zones) {
        cv::Scalar colour;
        if (zone.type() == ZoneType::ENTRANCE)
            colour = cv::Scalar(0, 255, 0); // green
        else if (zone.type() == ZoneType::EXIT)
            colour = cv::Scalar(0, 0, 255); // red
        else if (zone.type() == ZoneType::CHOKEPOINT)
            colour = cv::Scalar(0, 165, 255); // orange
        else
            colour = cv::Scalar(255, 0, 0); // blue (GENERAL)

        // draw the polygon outline
        const std::vector<cv::Point>& poly = zone.polygon();
        cv::polylines(frame, poly, true, colour, 2);

        // compute centroid by averaging all point coordinates
        int cx = 0, cy = 0;
        for (const cv::Point& p : poly) {
            cx += p.x;
            cy += p.y;
        }
        if (!poly.empty()) {
            cx /= static_cast<int>(poly.size());
            cy /= static_cast<int>(poly.size());
        }

        // find the matching metrics entry for this zone
        const CrowdMetrics* match = nullptr;
        for (const CrowdMetrics& m : metrics) {
            if (m.zoneName == zone.name()) {
                match = &m;
                break;
            }
        }

        if (match != nullptr) {
            char label[128];
            std::snprintf(label, sizeof(label), "%s  %.0f%%  %dp",
                zone.name().c_str(),
                match->density * 100.0f,
                match->count);

            cv::putText(frame, label, cv::Point(cx, cy),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

            // draw flow arrow if there is meaningful movement
            if (match->flowMagnitude >= 0.5f) {
                double angle_rad = match->flowAngle * CV_PI / 180.0;
                cv::Point end(
                    static_cast<int>(cx + std::cos(angle_rad) * match->flowMagnitude * 5),
                    static_cast<int>(cy + std::sin(angle_rad) * match->flowMagnitude * 5)
                );
                cv::arrowedLine(frame, cv::Point(cx, cy), end, colour, 2);
            }
        }
    }

    // alert banner
    std::optional<AlertEvent> alert;
    std::chrono::steady_clock::time_point receivedAt;
    {
        std::lock_guard<std::mutex> lock(alertMutex_);
        alert = latestAlert_;
        receivedAt = alertReceivedAt_;
    }

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (alert.has_value() && (now - receivedAt) < kBannerDuration) {
        // filled red bar across the top 35 pixels
        cv::rectangle(frame, cv::Point(0, 0), cv::Point(frame.cols, 35),
            cv::Scalar(0, 0, 200), cv::FILLED);

        // convert AlertType to a short string
        const char* typeStr = "ALERT";
        if (alert->type == AlertType::CONGESTION)       typeStr = "CONGESTION";
        else if (alert->type == AlertType::CHOKEPOINT)  typeStr = "CHOKEPOINT";
        else if (alert->type == AlertType::FLOW_REVERSAL) typeStr = "FLOW REVERSAL";

        char banner[256];
        std::snprintf(banner, sizeof(banner), "[%s] %s", typeStr, alert->message.c_str());

        cv::putText(frame, banner, cv::Point(5, 25),
            cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
    }
}

}
