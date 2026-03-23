cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 20)

project(AIDetection_test VERSION 0.0.1 LANGUAGES CXX)

find_package(OpenCV REQUIRED)

add_executable(test_hog test_hog.cpp ../hog-detection/HOGDetector.cpp)

target_include_directories(test_hog PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(test_hog PRIVATE ${OpenCV_LIBS} pthread)