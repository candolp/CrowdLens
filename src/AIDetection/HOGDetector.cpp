#include "HOGDetector.h"

HOGDetector::HOGDetector() : running(false), person_count(0) {
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
}

HOGDetector::~HOGDetector() {
    stop();
}

void HOGDetector::setCountCallback(std::function<void(int)> callback) {
    on_count_updated = callback;
}

int HOGDetector::getPersonCount() const {
    return person_count;
}

bool HOGDetector::start(const std::string& video_source) {
    cap.open(video_source);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open video source" << std::endl;
        return false;
    }
    running = true;
    detection_thread = std::thread(&HOGDetector::detectionLoop, this);
    return true;
}

void HOGDetector::updateFrame(cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(frame_mutex);
    frame.copyTo(current_frame);
}

void HOGDetector::stop() {
    running = false;
    if (detection_thread.joinable())
        detection_thread.join();
    cap.release();
    cv::destroyAllWindows();
}

void HOGDetector::detectionLoop() {
    cv::Mat frame, small_frame;
    int frame_count = 0;

    while (running) {
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if (current_frame.empty()) continue;
            current_frame.copyTo(frame);
        }

        cv::resize(frame, small_frame, cv::Size(640, 480));

        if (frame_count % 5 == 0) {
            std::vector<cv::Rect> rects;
            std::vector<double> weights;

            hog.detectMultiScale(small_frame, rects, weights,
                                 0, cv::Size(8,8), cv::Size(4,4), 1.05);

            person_count = rects.size();

            if (on_count_updated) {
                on_count_updated(person_count);
            }

            for (const auto& r : rects)
                cv::rectangle(small_frame, r, cv::Scalar(0, 255, 0), 2);
        }

        cv::putText(small_frame, "People: " + std::to_string(person_count),
                    cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
                    0.7, cv::Scalar(0, 255, 255), 2);

        cv::imshow("HOG Detection", small_frame);
        cv::waitKey(1);
        frame_count++;
    }
}