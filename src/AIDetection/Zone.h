#pragma once

#include <optional>
#include <string>
#include <vector>
#include <opencv2/core.hpp>

namespace cl {

// how a zon is used in the scene
enum class ZoneType {
    GENERAL,
    ENTRANCE,
    EXIT,
    CHOKEPOINT
};

// named polygon region in the camera frame
class Zone {
public:
    Zone(const std::string& name, const std::vector<cv::Point>& polygon, ZoneType type);

    const std::string& name() const;
    const std::vector<cv::Point>& polygon() const;
    ZoneType type() const;

    // returns true if point is inside or on the edge of this zone
    bool contains(cv::Point point) const;

    // per-zone threshold overrides, empty means "use the global value"
    std::optional<float> densityThreshold() const;
    std::optional<float> chokepointThreshold() const;
    std::optional<float> flowMagnitudeThreshold() const;
    std::optional<int> pixelsPerPerson() const;

    void setDensityThreshold(float v);
    void setChokepointThreshold(float v);
    void setFlowMagnitudeThreshold(float v);
    void setPixelsPerPerson(int v);

private:
    std::string name_;
    std::vector<cv::Point> polygon_;
    ZoneType type_;

    std::optional<float> densityThreshold_;
    std::optional<float> chokepointThreshold_;
    std::optional<float> flowMagnitudeThreshold_;
    std::optional<int> pixelsPerPerson_;
};

}
