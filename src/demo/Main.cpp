//
// Created by candolp on 20/02/2026.
//

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
#include <sstream>
#include <opencv2/core.hpp>

cl::ZoneType zoneTypeFromString(const std::string& s);

int main(int argc, char* argv[]) {
    std::string configPath = "src/config.yaml";
    if (argc > 1) configPath = argv[1];

    ConfigLoader loader;
    loader.load(configPath);
    int cameraIndex = std::stoi(loader.getValue("Camera:index", "0"));
    std::string videoFilePath = loader.getValue("Camera:video_file", "");

    // parse zone names (comma-separated) and build zones directly
    cl::ZoneManager zoneManager;
    std::string namesStr = loader.getValue("zones:names", "");
    std::istringstream namesStream(namesStr);
    std::string zoneName;
    while (std::getline(namesStream, zoneName, ',')) {
        size_t start = zoneName.find_first_not_of(" \t");
        size_t end   = zoneName.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        zoneName = zoneName.substr(start, end - start + 1);

        std::string type = loader.getValue("zones:zone." + zoneName + ":type", "GENERAL");
        std::string pointsStr = loader.getValue("zones:zone." + zoneName + ":points", "");
        std::istringstream pointsStream(pointsStr);
        std::string token;
        std::vector<cv::Point> poly;
        while (pointsStream >> token) {
            size_t comma = token.find(',');
            if (comma != std::string::npos)
                poly.emplace_back(std::stoi(token.substr(0, comma)), std::stoi(token.substr(comma + 1)));
        }
        zoneManager.addZone(cl::Zone(zoneName, poly, zoneTypeFromString(type)));
    }

    // build processor and apply config values
    std::unique_ptr<cl::OpenCVFrameProcessor> processor = std::make_unique<cl::OpenCVFrameProcessor>();
    processor->setPixelsPerPerson(std::stoi(loader.getValue("frame_processor:pixels_per_person", "500")));
    processor->setPyrScale(std::stod(loader.getValue("optical_flow:pyr_scale", "0.5")));
    processor->setLevels(std::stoi(loader.getValue("optical_flow:levels", "3")));
    processor->setWinSize(std::stoi(loader.getValue("optical_flow:win_size", "15")));
    processor->setIterations(std::stoi(loader.getValue("optical_flow:iterations", "3")));
    processor->setPolyN(std::stoi(loader.getValue("optical_flow:poly_n", "5")));
    processor->setPolySigma(std::stod(loader.getValue("optical_flow:poly_sigma", "1.2")));

    // build analyser and apply thresholds from config
    cl::CrowdAnalyser analyser(std::move(processor), zoneManager);
    analyser.setDensityThreshold(std::stof(loader.getValue("thresholds:density", "0.7")));
    analyser.setChokepointThreshold(std::stof(loader.getValue("thresholds:chokepoint", "0.85")));
    analyser.setFlowMagnitudeThreshold(std::stof(loader.getValue("thresholds:flow_magnitude", "2.0")));

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
    std::unique_ptr<cl::IFrameSource> source;
    if (cameraIndex == -1)
        source = std::make_unique<cl::VideoFileFrameSource>(videoFilePath);
    else
        source = std::make_unique<cl::CameraFrameSource>(cameraIndex);

    source->setFrameCallback([&analyser](cv::Mat frame, std::chrono::steady_clock::time_point ts) {
        analyser.onFrameArrived(std::move(frame), ts);
    });

    overlay.start();
    analyser.run(TrafficState::NO_TRAFFIC);
    source->start();

    std::cout << "CrowdLens running. Press 'q' in the window to stop.\n";
    while (overlay.tick()) {}

    // shutdown in reverse dependency order: capture -> analyser -> overlay
    source->stop();
    analyser.stop(TrafficState::NO_TRAFFIC);
    overlay.stop();
    return 0;
}

cl::ZoneType zoneTypeFromString(const std::string& s) {
    if (s == "ENTRANCE") return cl::ZoneType::ENTRANCE;
    if (s == "EXIT") return cl::ZoneType::EXIT;
    if (s == "CHOKEPOINT") return cl::ZoneType::CHOKEPOINT;
    return cl::ZoneType::GENERAL;
}
