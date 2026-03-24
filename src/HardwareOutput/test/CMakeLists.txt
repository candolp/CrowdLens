cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 20)
enable_testing()
include_directories(..)


add_executable(BuzzerOutputTest
        BuzzerOutputTest.cpp
        ../BUZZEROutput.cpp
        ../LEDOutput.cpp
        ../GPIODigitalOutput.cpp
)

target_link_libraries(BuzzerOutputTest Common gpiodcxx)
add_test(NAME TestBuzzerOutput COMMAND BuzzerOutputTest)

add_executable(LEDOutputTest
        LEDOutputTest.cpp
        ../LEDOutput.cpp
        ../BUZZEROutput.cpp
        ../GPIODigitalOutput.cpp
)
target_link_libraries(LEDOutputTest Common gpiodcxx)
add_test(NAME TestLEDOutput COMMAND LEDOutputTest)
