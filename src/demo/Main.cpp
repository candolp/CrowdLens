//
// Created by candolp on 20/02/2026.
//

#include <iostream>
#include <string>
#include <stdio.h>
#include <thread>
#include <chrono>

#include "../HardwareOutput/BuzzerOutput.h"
#include "../HardwareOutput/LEDOutput.h"
#include "../IRSensor/IRSensor.h"

#include "../Common/ConfigLoader.h"
#include "../CameraCapture/CameraFrameSource.h"
#include "../CameraCapture/VideoFileFrameSource.h"
#include "../AIDetection/CrowdAnalyser.h"
#include "../AIDetection/OpenCVFrameProcessor.h"
#include "../AIDetection/Zone.h"
#include "../AIDetection/ZoneManager.h"
#include "../Common/CrowdLensState.h"
#include "../Notification/ConsoleEventHandler.h"
#include "../Display/FrameOverlay.h"
#include <iostream>
#include <memory>
#include <opencv2/core.hpp>

int main() {
#ifdef CONFIG_PATH
    ConfigLoader config(CONFIG_PATH);
#else
    ConfigLoader config("config.yaml");
#endif

    std::cout << "CrowdLense is Running ..." << std::endl;
    LEDOutput greenLED  = LEDOutput(22, 0, TrafficState::TRAFFIC);
    LEDOutput redLED  = LEDOutput(10, 0,TrafficState::STAMPEDE);
    // BUZZEROutput trafficBuzzer = BUZZEROutput(config, 18, TrafficState::TRAFFIC);
    // trafficBuzzer.setBuzzerFrequency(1500);
    // trafficBuzzer.setBuzzerBeatsPerCycle(2);
    // BUZZEROutput stampedeBuzzer = BUZZEROutput(config, 18, TrafficState::STAMPEDE);
    // stampedeBuzzer.setBuzzerFrequency(1500);
    // stampedeBuzzer.setBuzzerBeatsPerCycle(5);
    // IRSensor isensor(config);
    // isensor.registerEventRunnable(greenLED);
    // isensor.registerEventRunnable(redLED);
    // isensor.registerEventRunnable(trafficBuzzer);
    // isensor.run(TrafficState::NO_TRAFFIC);
    redLED.run(TrafficState::STAMPEDE);
    greenLED.run(TrafficState::TRAFFIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    // printf("Press enter\n");
    // getchar();
    greenLED.stop(TrafficState::NO_TRAFFIC);
    redLED.stop(TrafficState::NO_TRAFFIC);
    // isensor.stop(TrafficState::NO_TRAFFIC);


    cl::ZoneManager zoneManager(config);
    std::unique_ptr<cl::OpenCVFrameProcessor> processor = std::make_unique<cl::OpenCVFrameProcessor>(config);
    cl::CrowdAnalyser analyser(std::move(processor), zoneManager, config);

    // register alert subscribers
    cl::ConsoleEventHandler console;
    analyser.registerAlertRunnable(console);

    cl::FrameOverlay overlay("CrowdLens");
    analyser.registerAlertRunnable(overlay);

    analyser.setDisplayCallback(
        [&overlay](cv::Mat frame, std::vector<cl::CrowdMetrics> metrics, std::vector<cl::Zone> zones) {
            overlay.pushFrame(std::move(frame), std::move(metrics), std::move(zones));
        }
    );

    // pick frame source based on config
    int cameraIndex = std::stoi(config.getValue("Camera:index", "0"));
    std::unique_ptr<cl::IFrameSource> source;
    if (cameraIndex == -1)
    {
        std::string videoFilePath = config.getValue("Camera:video_file", "");
        source = std::make_unique<cl::VideoFileFrameSource>(videoFilePath);
    }
    else
        source = std::make_unique<cl::CameraFrameSource>(cameraIndex);

    source->setFrameCallback([&analyser](cv::Mat frame, std::chrono::steady_clock::time_point ts) {
        analyser.onFrameArrived(std::move(frame), ts);
    });

    overlay.start();
    analyser.run(TrafficState::NO_TRAFFIC);
    source->start();

    std::cout << "CrowdLens running. Press 'q' in the window to stop.\n";
    overlay.runUntilClosed();

    // shutdown in reverse dependency order: capture -> analyser -> overlay
    source->stop();
    analyser.stop(TrafficState::NO_TRAFFIC);
    overlay.stop();
    return 0;
}
