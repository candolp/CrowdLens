#include "Zone.h"

#include <opencv2/imgproc.hpp>

namespace cl {

Zone::Zone(const std::string& name, const std::vector<cv::Point>& polygon, ZoneType type) {
    name_ = name;
    polygon_ = polygon;
    type_ = type;
}

const std::string& Zone::name() const { return name_; }
const std::vector<cv::Point>& Zone::polygon() const { return polygon_; }
ZoneType Zone::type() const { return type_; }

bool Zone::contains(cv::Point point) const {
    if (polygon_.empty()) return false;
    return cv::pointPolygonTest(polygon_, cv::Point2f(point), false) >= 0;
}

std::optional<float> Zone::densityThreshold() const { return densityThreshold_; }
std::optional<float> Zone::chokepointThreshold() const { return chokepointThreshold_; }
std::optional<float> Zone::flowMagnitudeThreshold() const { return flowMagnitudeThreshold_; }
std::optional<int> Zone::pixelsPerPerson() const { return pixelsPerPerson_; }

void Zone::setDensityThreshold(float v) {
    if (v >= 0.0f && v <= 1.0f) densityThreshold_ = v;
}

void Zone::setChokepointThreshold(float v) {
    if (v >= 0.0f && v <= 1.0f) chokepointThreshold_ = v;
}

void Zone::setFlowMagnitudeThreshold(float v) {
    if (v >= 0.0f) flowMagnitudeThreshold_ = v;
}

void Zone::setPixelsPerPerson(int v) {
    if (v > 0) pixelsPerPerson_ = v;
}
}
