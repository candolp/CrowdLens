cmake_minimum_required(VERSION 3.14)
project(CrowdLens VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_compile_options(-Wall -Wconversion -Wextra -pedantic)

# Layer subdirectories (existing code)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/capture)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/processing)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/analysis)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/display)

# Team subdirectories (to be populated)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/AIDetection)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/CameraCapture)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/HardwareOutput)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/IRSensor)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Notification)

# Entry point
add_subdirectory(${CMAKE_SOURCE_DIR}/src/demo)

# Tests
enable_testing()
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
