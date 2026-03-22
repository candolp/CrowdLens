#include <gtest/gtest.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include "../CrowdAnalyser.h"
#include "../IFrameProcessor.h"
#include "../ZoneManager.h"
#include "../Zone.h"
#include "../../Common/CrowdMetrics.h"
#include "../../Common/AlertRunnable.h"

namespace {

// returns whatever metrics the test gives it
class StubProcessor : public cl::IFrameProcessor {
public:
    std::vector<cl::CrowdMetrics> fixedMetrics_;

    std::vector<cl::CrowdMetrics> processFrame(
        const cv::Mat&,
        const std::vector<cl::Zone>&,
        std::chrono::steady_clock::time_point) override
    {
        return fixedMetrics_;
    }
};

// blocks inside processFrame until the test opens the gate
// records the first-pixel value of every frame it processes
class BlockingStubProcessor : public cl::IFrameProcessor {
public:
    std::mutex mx_;
    std::condition_variable cv_;
    bool gateOpen_ = false;
    bool blocking_ = false;
    int  callCount_ = 0;
    std::vector<int> seenMarkers_;

    std::vector<cl::CrowdMetrics> processFrame(
        const cv::Mat& frame,
        const std::vector<cl::Zone>&,
        std::chrono::steady_clock::time_point) override
    {
        std::unique_lock<std::mutex> lk(mx_);
        blocking_ = true;
        cv_.notify_all();
        cv_.wait(lk, [this]{ return gateOpen_; });
        seenMarkers_.push_back(frame.at<uchar>(0, 0));
        ++callCount_;
        cv_.notify_all();
        return {};
    }

    // blocks the test until processFrame is blocking on the gate
    void waitUntilBlocking() {
        std::unique_lock<std::mutex> lk(mx_);
        cv_.wait_for(lk, std::chrono::seconds(2), [this]{ return blocking_; });
    }

    // releases the gate so all future (and current) processFrame calls pass through
    void openGate() {
        std::lock_guard<std::mutex> lk(mx_);
        gateOpen_ = true;
        cv_.notify_all();
    }

    // blocks the test until at least n calls have completed.
    void waitForCallCount(int n) {
        std::unique_lock<std::mutex> lk(mx_);
        cv_.wait_for(lk, std::chrono::seconds(2), [this, n]{ return callCount_ >= n; });
    }
};

// minimal AlertRunnable that captures the first fired event
class MockAlertRunnable : public cl::AlertRunnable {
public:
    std::function<void(cl::AlertEvent)> handler;
    void onAlert(const cl::AlertEvent& event) override {
        if (handler) handler(event);
    }
    void stop() override {}
};

}

TEST(CrowdAnalyser, StartsAndStopsCleanly) {
    cl::ZoneManager manager;
    std::unique_ptr<StubProcessor> stub = std::make_unique<StubProcessor>();
    cl::CrowdAnalyser analyser(std::move(stub), manager);
    analyser.run(cl::TrafficState::NO_TRAFFIC);
    analyser.stop();
    SUCCEED();
}

TEST(CrowdAnalyser, AlertCallbackFiredWhenThresholdExceeded) {
    cl::ZoneManager manager;
    // add a zone so the metrics have a matching name
    manager.addZone(cl::Zone("zone1", {{0,0},{100,0},{100,100},{0,100}}, cl::ZoneType::GENERAL));

    std::unique_ptr<StubProcessor> stub = std::make_unique<StubProcessor>();
    cl::CrowdMetrics metrics;
    metrics.zoneName = "zone1";
    metrics.density = 0.9f; // above the default 0.7 threshold
    metrics.count = 10;
    metrics.flowAngle = 0.0f;
    metrics.flowMagnitude = 3.0f; // above 2.0 so only CONGESTION fires, not CHOKEPOINT
    metrics.timestamp = std::chrono::steady_clock::now();
    stub->fixedMetrics_ = {metrics};

    cl::CrowdAnalyser analyser(std::move(stub), manager);

    std::mutex cbMutex;
    std::condition_variable cbCv;
    std::atomic<bool> callbackFired{false};
    cl::AlertType firedType{};

    MockAlertRunnable mock;
    mock.handler = [&](cl::AlertEvent event) {
        firedType = event.type;
        callbackFired.store(true);
        cbCv.notify_one();
    };
    analyser.registerAlertRunnable(mock);

    analyser.run(cl::TrafficState::NO_TRAFFIC);
    analyser.onFrameArrived(cv::Mat::zeros(100, 100, CV_8UC1), std::chrono::steady_clock::now());

    std::unique_lock<std::mutex> lk(cbMutex);
    bool signalled = cbCv.wait_for(lk, std::chrono::seconds(2), [&]{ return callbackFired.load(); });
    analyser.stop();

    EXPECT_TRUE(signalled);
    EXPECT_EQ(firedType, cl::AlertType::CONGESTION);
}

TEST(CrowdAnalyser, DropOldestPolicyKeepsFreshestFrame) {
    cl::ZoneManager manager;

    // keep a raw pointer so we can inspect it after the analyser is stopped
    BlockingStubProcessor* rawPtr = new BlockingStubProcessor();
    std::unique_ptr<cl::IFrameProcessor> stub = std::unique_ptr<cl::IFrameProcessor>(rawPtr);

    cl::CrowdAnalyser analyser(std::move(stub), manager);
    analyser.run(cl::TrafficState::NO_TRAFFIC);

    // send frame0, the analysis thread picks it up and blocks inside processFrame
    cv::Mat frame0(1, 1, CV_8UC1, cv::Scalar(0));
    analyser.onFrameArrived(frame0, std::chrono::steady_clock::now());
    rawPtr->waitUntilBlocking();

    // push two frames while the thread is busy
    // frameA is immediately overwritten by frameB (drop oldest)
    cv::Mat frameA(1, 1, CV_8UC1, cv::Scalar(42));
    cv::Mat frameB(1, 1, CV_8UC1, cv::Scalar(99));
    analyser.onFrameArrived(frameA, std::chrono::steady_clock::now());
    analyser.onFrameArrived(frameB, std::chrono::steady_clock::now());

    // release the gate, frame0 finishes, then frameB is processed
    rawPtr->openGate();
    rawPtr->waitForCallCount(2);

    analyser.stop();

    ASSERT_EQ(static_cast<int>(rawPtr->seenMarkers_.size()), 2);
    EXPECT_EQ(rawPtr->seenMarkers_[0], 0); // frame0
    EXPECT_EQ(rawPtr->seenMarkers_[1], 99); // frameB — frameA was dropped
}
