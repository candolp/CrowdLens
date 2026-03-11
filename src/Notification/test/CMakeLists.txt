cmake_minimum_required(VERSION 3.6)
project(Notification_test VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(test_email TestEmail.cpp)

# Pull in the EmailNotification library and its headers from the parent
target_link_libraries(test_email PRIVATE Notification_lib)

# Register with CTest so `ctest` or `cmake --build . --target test` works
add_test(NAME EmailNotifierTests COMMAND test_email)

# send test
target_compile_definitions(test_email PRIVATE LIVE_TEST)