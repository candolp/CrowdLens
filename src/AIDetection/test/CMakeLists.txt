cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

project(AIDetection_test)

find_package(OpenCV REQUIRED)

add_executable(test_hog 
    test_hog.cpp 
    ../HOGDetector.cpp
)

target_include_directories(test_hog PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    ..
)

target_link_libraries(test_hog PRIVATE ${OpenCV_LIBS} pthread)