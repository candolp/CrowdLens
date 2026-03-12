#include <gtest/gtest.h>
#include "../CameraFrameSource.h"
#include "../VideoFileFrameSource.h"
#include <chrono>

// skip on macOS, run normally on linux, necessary for local testing
#ifdef __APPLE__
static constexpr bool kSkipOnMac = true;
#else
static constexpr bool kSkipOnMac = false;
#endif

// device index -1 won't open on Linux, safe for CI
TEST(CameraFrameSource, StartsAndStopsWithoutCrashing) {
    if (kSkipOnMac) GTEST_SKIP() << "AVFoundation requires main thread; runs on Linux CI";

    cl::CameraFrameSource source(-1);
    source.start();
    source.stop(); // captureLoop exits early since device fails to open, thread joins cleanly
    SUCCEED();
}

TEST(CameraFrameSource, CallbackNotInvokedWhenDeviceFailsToOpen) {
    if (kSkipOnMac) GTEST_SKIP() << "AVFoundation requires main thread; runs on Linux CI";

    cl::CameraFrameSource source(-1);

    // flip this flag if the callback ever fires
    bool callbackFired = false;
    source.setFrameCallback([&](cv::Mat, std::chrono::steady_clock::time_point) {
        callbackFired = true;
    });

    source.start();
    source.stop(); // stop() joins the thread, so callbackFired is safe to read after this

    EXPECT_FALSE(callbackFired);
}

// VideoFileFrameSource tests
// A non-existent path should cause captureLoop to return early without crashing
TEST(VideoFileFrameSource, FileNotFoundDoesNotCrash) {
    cl::VideoFileFrameSource source("nonexistent_file_that_does_not_exist.mp4");
    source.start();
    source.stop();
    SUCCEED();
}

TEST(VideoFileFrameSource, CallbackNotInvokedWhenFileNotFound) {
    cl::VideoFileFrameSource source("nonexistent_file_that_does_not_exist.mp4");

    bool callbackFired = false;
    source.setFrameCallback([&](cv::Mat, std::chrono::steady_clock::time_point) {
        callbackFired = true;
    });

    source.start();
    source.stop();

    EXPECT_FALSE(callbackFired);
}
