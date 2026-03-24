cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

add_library(HardwareOutput LEDOutput.cpp
        BUZZEROutput.cpp
        GPIODigitalOutput.cpp
)
target_link_libraries(HardwareOutput Common gpiodcxx)
add_subdirectory(test)
