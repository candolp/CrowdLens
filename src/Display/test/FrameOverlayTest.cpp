#include <gtest/gtest.h>
#include "../FrameOverlay.h"
#include "../../Common/AlertEvent.h"
#include <opencv2/core.hpp>


TEST(FrameOverlay, TickReturnsFalseBeforeStart) {
    cl::FrameOverlay overlay("test_window");
    EXPECT_FALSE(overlay.tick());
}

TEST(FrameOverlay, TickReturnsFalseAfterStop) {
    cl::FrameOverlay overlay("test_window");
    overlay.start();
    overlay.stop();
    EXPECT_FALSE(overlay.tick());
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
