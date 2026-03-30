add_library(Notification
    ConsoleEventHandler.cpp
)

target_include_directories(Notification PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(Notification PUBLIC Common)

add_subdirectory(test)
