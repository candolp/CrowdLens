#include "../HOGDetector.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

// Test 1 - video file opens correctly
void testVideoOpens() {
    HOGDetector detector;
    bool result = detector.start("video1.mp4");
    assert(result == true);
    std::cout << "PASS: Video opens successfully" << std::endl;
    detector.stop();
}

// Test 2 - invalid file fails gracefully
void testInvalidVideoFails() {
    HOGDetector detector;
    bool result = detector.start("nonexistent.mp4");
    assert(result == false);
    std::cout << "PASS: Invalid video handled correctly" << std::endl;
}

// Test 3 - callback fires when frames are processed
void testCallbackFires() {
    HOGDetector detector;
    bool callback_fired = false;

    detector.setCountCallback([&callback_fired](int count) {
        callback_fired = true;
    });

    detector.start("video1.mp4");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    detector.stop();

    assert(callback_fired == true);
    std::cout << "PASS: Callback fired correctly" << std::endl;
}

// Test 4 - person count is non negative
void testPersonCountValid() {
    HOGDetector detector;

    detector.setCountCallback([](int count) {
        assert(count >= 0);
    });

    detector.start("video1.mp4");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    detector.stop();

    std::cout << "PASS: Person count always non-negative" << std::endl;
}

// Test 5 - thread safety, no crash under rapid updates
void testThreadSafety() {
    HOGDetector detector;
    detector.start("video1.mp4");

    cv::VideoCapture cap("video1.mp4");
    cv::Mat frame;
    int updates = 0;

    while (updates < 50) {
        cap >> frame;
        if (frame.empty()) break;
        detector.updateFrame(frame);
        updates++;
    }

    detector.stop();
    std::cout << "PASS: No crash under rapid frame updates" << std::endl;
}

int main() {
    std::cout << "Running HOGDetector tests..." << std::endl;
    std::cout << "-------------------------------" << std::endl;

    testVideoOpens();
    testInvalidVideoFails();
    testCallbackFires();
    testPersonCountValid();
    testThreadSafety();

    std::cout << "-------------------------------" << std::endl;
    std::cout << "All tests passed" << std::endl;
    return 0;
}