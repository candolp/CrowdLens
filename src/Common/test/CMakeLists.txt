cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
enable_testing()
include_directories(..)


add_executable(ConfigLoaderTest ConfigLoaderTest.cpp )
target_link_libraries(ConfigLoaderTest  Common)
add_test(NAME TestCommonConfigLoader COMMAND ConfigLoaderTest)

target_compile_definitions(ConfigLoaderTest PRIVATE
        CONFIG_PATH="${CMAKE_SOURCE_DIR}/src/config.yaml")

