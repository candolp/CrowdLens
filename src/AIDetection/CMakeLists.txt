find_package(OpenCV REQUIRED)

add_library(AIDetection
      Zone.cpp
      ZoneManager.cpp
      OpenCVFrameProcessor.cpp
      CrowdAnalyser.cpp
      StampedePredictor.cpp
)

target_include_directories(AIDetection PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(AIDetection PUBLIC ${OpenCV_LIBS} Common)

add_subdirectory(test)
