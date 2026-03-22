find_package(OpenCV REQUIRED)

add_library(CameraCapture
    CameraFrameSource.cpp
    VideoFileFrameSource.cpp
)

target_include_directories(CameraCapture PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(CameraCapture PUBLIC ${OpenCV_LIBS})

add_subdirectory(test)
