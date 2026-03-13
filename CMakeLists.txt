cmake_minimum_required(VERSION 3.16)
project(CrowdLens VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

enable_testing()


include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#add subdirectory containing demo code
add_subdirectory(${CMAKE_SOURCE_DIR}/src/demo)

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

#add subdirectory containing visual overlay / display window
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Display)

#add subdirectory containing common codes
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Common)

##compile options to check the code
add_compile_options(-Wall -Wconversion -Wextra -pedantic)
