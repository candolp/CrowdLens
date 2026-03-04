cmake_minimum_required(VERSION 3.31.6)
project(CrowdLens VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

#add subdirectory containing common codes
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Common)

#add subdirectory containing AI code for crowd detection
add_subdirectory(${CMAKE_SOURCE_DIR}/src/AIDetection)

#add subdirectory containing codes for capturing video from camera
add_subdirectory(${CMAKE_SOURCE_DIR}/src/CameraCapture)

#add subdirectory containing code for hardware output: speaker and led
add_subdirectory(${CMAKE_SOURCE_DIR}/src/HardwareOutput)

#add subdirectory containing code for Infer-red sensor
add_subdirectory(${CMAKE_SOURCE_DIR}/src/IRSensor)

#add subdirectory containing sending notification to stakeholders
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Notification)

#add subdirectory containing demo code
add_subdirectory(${CMAKE_SOURCE_DIR}/src/demo)

##compile options to check the code
add_compile_options(-Wall -Wconversion -Wextra -pedantic)
