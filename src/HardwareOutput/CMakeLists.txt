add_library(LEDOutput LEDOutput.cpp)
target_link_libraries(LEDOutput Common gpiodcxx)
add_subdirectory(test)
