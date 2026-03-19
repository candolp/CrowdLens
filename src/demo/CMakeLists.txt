cmake_minimum_required(VERSION 3.16)
project(CrowdLens)

set(CMAKE_CXX_STANDARD 20)

add_executable(CrowdLens
        Main.cpp
)

target_link_libraries(CrowdLens
        Common
        pthread)
