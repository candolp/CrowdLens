#include "OpenCVFrameProcessor.h"
#include "../Common/ConfigLoader.h"
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc.hpp>

namespace cl {

OpenCVFrameProcessor::OpenCVFrameProcessor() {
    bgSubtractor_ = cv::createBackgroundSubtractorMOG2();
}

OpenCVFrameProcessor::OpenCVFrameProcessor(const ConfigLoader& config) {
    bgSubtractor_ = cv::createBackgroundSubtractorMOG2();
    loadConfig(config);
}

void OpenCVFrameProcessor::loadConfig(const ConfigLoader& config) {
    setPixelsPerPerson(std::stoi(config.getValue("frame_processor:pixels_per_person", "500")));
    setPyrScale(std::stod(config.getValue("optical_flow:pyr_scale", "0.5")));
    setLevels(std::stoi(config.getValue("optical_flow:levels", "3")));
    setWinSize(std::stoi(config.getValue("optical_flow:win_size", "15")));
    setIterations(std::stoi(config.getValue("optical_flow:iterations", "3")));
    setPolyN(std::stoi(config.getValue("optical_flow:poly_n", "5")));
    setPolySigma(std::stod(config.getValue("optical_flow:poly_sigma", "1.2")));
}

void OpenCVFrameProcessor::setPixelsPerPerson(int pixels) {
    if (pixels <= 0) return;
    pixelsPerPerson_ = pixels;
}

void OpenCVFrameProcessor::setPyrScale(double scale) {
    if (scale <= 0.0 || scale >= 1.0) return;
    pyrScale_ = scale;
}
void OpenCVFrameProcessor::setLevels(int levels) {
    if (levels <= 0) return;
    levels_ = levels;
}
void OpenCVFrameProcessor::setWinSize(int winSize) {
    if (winSize <= 0) return;
    winSize_ = winSize;
}
void OpenCVFrameProcessor::setIterations(int iterations) {
    if (iterations <= 0) return;
    iterations_ = iterations;
}
void OpenCVFrameProcessor::setPolyN(int polyN) {
    if (polyN <= 0) return;
    polyN_ = polyN;
}
void OpenCVFrameProcessor::setPolySigma(double sigma) {
    if (sigma <= 0.0) return;
    polySigma_ = sigma;
}

std::vector<CrowdMetrics> OpenCVFrameProcessor::processFrame(
    const cv::Mat& frame,
    const std::vector<Zone>& zones,
    std::chrono::steady_clock::time_point ts)
{
    // greyscale & foreground mask
    cv::Mat grey;
    cv::cvtColor(frame, grey, cv::COLOR_BGR2GRAY);
    cv::Mat fgMask;
    bgSubtractor_->apply(grey, fgMask);

    // tracking how pixels move for movement analysis
    cv::Mat flow;
    if (!prevGrey_.empty())
    {
        cv::calcOpticalFlowFarneback(prevGrey_, grey, flow,
            pyrScale_, // pyramid scale
            levels_, // pyramid levels
            winSize_, // window size
            iterations_, // iterations
            polyN_,
            polySigma_,
            0 // flags
        );
    }

    std::vector<CrowdMetrics> results;
    for (const Zone& zone : zones)
    {
        // restricting analysis to zones by masks
        cv::Mat zoneMask = cv::Mat::zeros(frame.size(), CV_8UC1);
        const std::vector<cv::Point>& poly = zone.polygon();
        const cv::Point* pts = poly.data();
        int npts = static_cast<int>(poly.size());
        cv::fillPoly(zoneMask, &pts, &npts, 1, cv::Scalar(255));

        // calculate how full the zone is
        int zoneArea = cv::countNonZero(zoneMask);
        cv::Mat fgInZone;
        cv::bitwise_and(fgMask, zoneMask, fgInZone);
        int fgPixels = cv::countNonZero(fgInZone);
        float density = (zoneArea > 0) ? static_cast<float>(fgPixels) / zoneArea : 0.0f;

        // determine direction and speed inside the zone
        float flowAngle = 0.0f, flowMagnitude = 0.0f;
        if (!flow.empty())
        {
            cv::Scalar meanFlow = cv::mean(flow, zoneMask);
            float dx = static_cast<float>(meanFlow[0]);
            float dy = static_cast<float>(meanFlow[1]);
            flowMagnitude = std::sqrt(dx * dx + dy * dy);
            flowAngle = std::atan2(dy, dx) * 180.0f / static_cast<float>(M_PI);
            if (flowAngle < 0)
                flowAngle += 360.0f;
        }

        CrowdMetrics m;
        m.zoneName = zone.name();
        m.density = density;
        m.count = fgPixels / pixelsPerPerson_; // assuming each person occupies ~pixelsPerPerson_ pixels
        m.flowAngle = flowAngle;
        m.flowMagnitude = flowMagnitude;
        m.timestamp = ts;
        results.push_back(m);
    }

    prevGrey_ = grey.clone();
    return results;
}

}
