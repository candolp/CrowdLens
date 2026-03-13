#include <gtest/gtest.h>
#include "../FrameOverlay.h"
#include "../../Common/AlertEvent.h"
#include <opencv2/core.hpp>


TEST(FrameOverlay, RunUntilClosedReturnsImmediatelyIfNotStarted) {
    cl::FrameOverlay overlay("test_window");
    // no thread was started so runUntilClosed should return without blocking
    overlay.runUntilClosed();
    SUCCEED();
}

TEST(FrameOverlay, StopJoinsThreadCleanly) {
    cl::FrameOverlay overlay("test_window");
    overlay.start();
    overlay.stop();
    // thread was joined by stop(), runUntilClosed should return immediately
    overlay.runUntilClosed();
    SUCCEED();
}

TEST(FrameOverlay, PushFrameDoesNotCrash) {
    cl::FrameOverlay overlay("test_window");
    cv::Mat frame(10, 10, CV_8UC3);
    EXPECT_NO_THROW(overlay.pushFrame(frame, {}, {}));
}

TEST(FrameOverlay, OnAlertDoesNotCrash) {
    cl::FrameOverlay overlay("test_window");
    cl::AlertEvent event{
        TrafficState::CROWDED,
        cl::AlertType::CONGESTION,
        cl::AlertSeverity::WARNING,
        cl::CrowdMetrics{},
        "test alert"
    };
    EXPECT_NO_THROW(overlay.onAlert(event));
}

TEST(FrameOverlay, DestructorIsClean) {
    {
        cl::FrameOverlay overlay("test_window");
        overlay.start();
        overlay.stop();
    }
    SUCCEED();
}
