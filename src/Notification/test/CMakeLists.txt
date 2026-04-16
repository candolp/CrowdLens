cmake_minimum_required(VERSION 3.6)
project(Notification_test VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)


# ── standalone live send test (always built, sends a real email) ───────────
add_executable(live_email_test LiveEmailTest.cpp)

target_include_directories(live_email_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..       # → EmailNotification.h
    ${CMAKE_SOURCE_DIR}/src/Common       # → ConfigLoader.h, TrafficEventHandler.h
)

target_link_libraries(live_email_test PRIVATE Notification_lib Common)

target_compile_definitions(live_email_test PRIVATE
    CONFIG_PATH="${CMAKE_BINARY_DIR}/config.yaml"
)

# ── GTest-based dispatcher tests ──────────────────────────────────────────
add_executable(Notification_test AlertDispatcherTest.cpp)

target_include_directories(Notification_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/src/Common
)

target_link_libraries(Notification_test PRIVATE Notification_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(Notification_test)