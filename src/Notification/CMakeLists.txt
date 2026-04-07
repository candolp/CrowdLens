#add_subdirectory(test)

# Find libcurl
find_package(CURL REQUIRED)

# Build the EmailNotifier as a static library so both the main project
# and the test executable can link against it without recompiling
add_library(Notification_lib STATIC
    EmailNotification.cpp
        ConsoleEventHandler.cpp
)

target_include_directories(Notification_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/src/Common
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(Notification_lib PUBLIC
    Common
    ${CURL_LIBRARIES}
)

# Enable tests and pull in the test subdirectory
enable_testing()
add_subdirectory(test)