cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
add_library(Common
     ConfigLoader.cpp
    CppTimer.h
    CppTimerCallbackInterface.h
    Runnable.h
)
# Make headers available to targets that link against Common
target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)

target_compile_definitions(Common PRIVATE
        CONFIG_PATH="${CMAKE_SOURCE_DIR}/src/config.yaml")