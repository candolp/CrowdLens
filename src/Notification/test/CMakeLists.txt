cmake_minimum_required(VERSION 3.6)
project(Notification_test VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# ── unit / smoke tests (no real SMTP) ─────────────────────────────────────
add_executable(test_email TestEmail.cpp)

target_include_directories(test_email PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..         # → EmailNotification.h
    ${CMAKE_SOURCE_DIR}/src/Common         # → ConfigLoader.h, TrafficEventHandler.h
)

# Pull in the EmailNotification library and its headers from the parent
# Common_lib must be explicit here so CMake knows the build-order dependency
target_link_libraries(test_email PRIVATE Notification_lib Common)

# CONFIG_PATH is defined globally by the root CMakeLists, but make sure it's here too
target_compile_definitions(test_email PRIVATE
    CONFIG_PATH="${CMAKE_BINARY_DIR}/config.yaml"
)

# Register with CTest so `ctest` or `cmake --build . --target test` works
add_test(NAME EmailNotifierTests COMMAND test_email)

# ── live email test (opt-in: cmake .. -DLIVE_TEST=ON) ─────────────────────
if(LIVE_TEST)
    target_compile_definitions(test_email PRIVATE LIVE_TEST)
endif()

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