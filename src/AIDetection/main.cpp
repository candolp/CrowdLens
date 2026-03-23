#include "HOGDetector.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    HOGDetector detector;

    detector.setCountCallback([](int count) {
        std::cout << "People detected: " << count << std::endl;
    });

    // swap this line depending on what you're testing
    // detector.start("video1.mp4");   // recorded video
    detector.startCamera(0);           // live USB camera

    // run for 30 seconds then stop
    std::this_thread::sleep_for(std::chrono::seconds(30));

    detector.stop();
    return 0;
}