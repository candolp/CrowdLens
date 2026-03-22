find_package(OpenCV REQUIRED)

add_library(Display
    FrameOverlay.cpp
)

target_include_directories(Display PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(Display PUBLIC AIDetection Common ${OpenCV_LIBS})

add_executable(Display_test
    test/FrameOverlayTest.cpp
)
target_link_libraries(Display_test PRIVATE Display GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(Display_test)
