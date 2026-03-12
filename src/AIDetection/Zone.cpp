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
}
