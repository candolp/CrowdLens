#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <string>

class HOGDetector {
private:
    cv::HOGDescriptor hog;
    cv::VideoCapture cap;
    std::thread detection_thread;
    std::mutex frame_mutex;
    cv::Mat current_frame;
    std::atomic<bool> running;
    std::atomic<int> person_count;
    std::function<void(int)> on_count_updated;

    void detectionLoop();  // declared here, defined in .cpp

public:
    HOGDetector();         // constructor
    ~HOGDetector();        // destructor

    void setCountCallback(std::function<void(int)> callback);
    int getPersonCount() const;
    bool start(const std::string& video_source);
    bool startCamera(int device_index = 0);
    void updateFrame(cv::Mat& frame);
    void stop();
};