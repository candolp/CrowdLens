#include <gtest/gtest.h>
#include "../ConsoleEventHandler.h"
#include "../../Common/TrafficEventHandler.h"
#include "../../Common/AlertRunnable.h"
#include "../../Common/AlertEvent.h"

namespace {

// minimal AlertRunnable that counts calls
class CountingRunnable : public cl::AlertRunnable {
public:
    int alertCount_ = 0;
    int stopCount_ = 0;

    void onAlert(const cl::AlertEvent&) override { ++alertCount_; }
    void run(TrafficState /*state*/) override {}
    void stop(TrafficState /*state*/) override { ++stopCount_; }
};

// concrete TrafficEventHandler subclass that exposes protected methods for testing
class TestEventHandler : public TrafficEventHandler {
public:
    void worker() override {}

    void fireAlert(const cl::AlertEvent& ev) { alertCallback(ev); }
    void fireEvent(TrafficState s) { eventCallback(s); }
};

cl::AlertEvent makeEvent() {
    cl::CrowdMetrics metrics;
    metrics.zoneName = "test_zone";
    return {TrafficState::CROWDED, cl::AlertType::CONGESTION, cl::AlertSeverity::WARNING, metrics, "test alert"};
}

}

TEST(TrafficEventHandler, DispatchesToRegisteredAlertRunnable) {
    TestEventHandler handler;
    CountingRunnable sink;

    handler.registerAlertRunnable(sink);
    handler.fireAlert(makeEvent());

    EXPECT_EQ(sink.alertCount_, 1);
}

TEST(TrafficEventHandler, StopBroadcastsToSubscribers) {
    TestEventHandler handler;
    CountingRunnable sink;

    handler.registerAlertRunnable(sink);
    handler.stop(TrafficState::NO_TRAFFIC);

    EXPECT_EQ(sink.stopCount_, 1);
}
