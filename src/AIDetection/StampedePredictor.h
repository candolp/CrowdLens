#pragma once

#include "../Common/CrowdMetrics.h"

#include <chrono>
#include <deque>
#include <map>
#include <string>

namespace cl {

// tracks density/flow history per zone and predicts stampede or chokepoint conditions
class StampedePredictor {
public:
    struct Prediction {
        bool stampedeRisk = false;
        bool chokepointRisk = false;
        float timeToStampede = 0.0f; // seconds
        float timeToChokepoint = 0.0f; // seconds
    };

    // add the latest metrics sample for the zone it came from
    void update(const CrowdMetrics& m);

    // predict whether stampede/chokepoint conditions will be met within horizon_
    Prediction predict(const std::string& zoneName) const;

    void setWindowSize(size_t n);
    void setPredictionHorizon(float seconds);
    void setStampedeDensityThreshold(float t);
    void setChokepointDensityThreshold(float t);
    void setFlowMagnitudeThreshold(float t);
    void setMinTrendSlope(float slope);

private:
    struct Sample {
        float density;
        float flowMagnitude;
        std::chrono::steady_clock::time_point timestamp;
    };

    std::map<std::string, std::deque<Sample>> history_;

    size_t windowSize_ = 10;
    float horizon_ = 10.0f; // seconds ahead to look
    float stampedeDensityThreshold_ = 0.9f;
    float chokepointDensityThreshold_ = 0.85f;
    float flowMagnitudeThreshold_ = 2.0f;
    float minTrendSlope_ = 0.005f; // density/sec below this is treated as noise
};

}
