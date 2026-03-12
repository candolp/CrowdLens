#include <gtest/gtest.h>
#include "../Zone.h"
#include <opencv2/core.hpp>

TEST(Zone, ContainsReturnsFalseForEmptyPolygon) {
    cl::Zone zone("empty", {}, cl::ZoneType::GENERAL);
    EXPECT_FALSE(zone.contains(cv::Point(5, 5)));
}

TEST(Zone, ContainsPointInsideSquare) {
    std::vector<cv::Point> square = {
        {0, 0}, {10, 0}, {10, 10}, {0, 10}
    };
    cl::Zone zone("square", square, cl::ZoneType::GENERAL);
    EXPECT_TRUE(zone.contains(cv::Point(5, 5)));
}

TEST(Zone, RejectsPointOutsidePolygon) {
    std::vector<cv::Point> square = {
        {0, 0}, {10, 0}, {10, 10}, {0, 10}
    };
    cl::Zone zone("square", square, cl::ZoneType::GENERAL);
    EXPECT_FALSE(zone.contains(cv::Point(20, 20)));
}
