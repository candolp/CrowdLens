#include "StampedePredictor.h"

#include <algorithm>

namespace cl {

void StampedePredictor::update(const CrowdMetrics& m) {
    std::deque<Sample>& window = history_[m.zoneName];
    window.push_back({ m.density, m.flowMagnitude, m.timestamp });

    // keep at most windowSize_ samples
    while (window.size() > windowSize_)
        window.pop_front();
}

StampedePredictor::Prediction StampedePredictor::predict(const std::string& zoneName) const {
    Prediction result;

    std::map<std::string, std::deque<Sample>>::const_iterator it = history_.find(zoneName);
    if (it == history_.end()) return result;

    const std::deque<Sample>& samples = it->second;
    if (samples.size() < 2) return result;

    const Sample& first = samples.front();
    const Sample& last = samples.back();

    float elapsed = std::chrono::duration<float>(last.timestamp - first.timestamp).count();
    if (elapsed <= 0.0f) return result;

    float densitySlope = (last.density - first.density) / elapsed;
    float flowSlope = (last.flowMagnitude - first.flowMagnitude) / elapsed;

    // STAMPEDE_RISK: density rising fast enough to hit the threshold within horizon_
    if (densitySlope > minTrendSlope_) {
        float timeToStampede = (stampedeDensityThreshold_ - last.density) / densitySlope;
        if (timeToStampede > 0.0f && timeToStampede <= horizon_) {
            result.stampedeRisk = true;
            result.timeToStampede = timeToStampede;
        }
    }

    // CHOKEPOINT_PREDICTED: density rising and flow falling, both converge within horizon_
    if (densitySlope > minTrendSlope_ && flowSlope < 0.0f) {
        float timeToDensity = (chokepointDensityThreshold_ - last.density) / densitySlope;
        float timeToLowFlow = (last.flowMagnitude - flowMagnitudeThreshold_) / (-flowSlope);
        if (timeToDensity > 0.0f && timeToLowFlow > 0.0f) {
            float timeToChokepoint = std::max(timeToDensity, timeToLowFlow);
            if (timeToChokepoint <= horizon_) {
                result.chokepointRisk = true;
                result.timeToChokepoint = timeToChokepoint;
            }
        }
    }

    return result;
}

void StampedePredictor::setWindowSize(size_t n) {
    if (n < 2) return;
    windowSize_ = n;
    // trim any existing windows that are now too long
    for (auto& [name, window] : history_) {
        while (window.size() > windowSize_)
            window.pop_front();
    }
}

void StampedePredictor::setPredictionHorizon(float seconds) {
    if (seconds > 0.0f) horizon_ = seconds;
}

void StampedePredictor::setStampedeDensityThreshold(float t) {
    if (t >= 0.0f && t <= 1.0f) stampedeDensityThreshold_ = t;
}

void StampedePredictor::setChokepointDensityThreshold(float t) {
    if (t >= 0.0f && t <= 1.0f) chokepointDensityThreshold_ = t;
}

void StampedePredictor::setFlowMagnitudeThreshold(float t) {
    if (t >= 0.0f) flowMagnitudeThreshold_ = t;
}

void StampedePredictor::setMinTrendSlope(float slope) {
    if (slope >= 0.0f) minTrendSlope_ = slope;
}

}
