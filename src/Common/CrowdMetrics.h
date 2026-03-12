#pragma once

#include <chrono>
#include <string>

namespace cl {

struct CrowdMetrics {
    std::string zoneName;
    float density; // ratio of detected pixels to zone area (0.0 - 1.0)
    int count; // estimated number of people in the zone
    float flowAngle; // direction of movement in degrees
    float flowMagnitude; // speed of movement in pixels/frame
    std::chrono::steady_clock::time_point timestamp;
};

}
