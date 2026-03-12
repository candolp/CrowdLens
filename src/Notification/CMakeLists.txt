#add_subdirectory(test)
cmake_minimum_required(VERSION 3.6)
project(Notification VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# Find libcurl
find_package(CURL REQUIRED)

# Build the EmailNotifier as a static library so both the main project
# and the test executable can link against it without recompiling
add_library(Notification_lib STATIC
    EmailNotification.cpp
    config/ConfigLoader.cpp
)

target_include_directories(Notification_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(Notification_lib PUBLIC
    ${CURL_LIBRARIES}
)

# Enable tests and pull in the test subdirectory
enable_testing()
add_subdirectory(test)