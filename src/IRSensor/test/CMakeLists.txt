#cmake_minimum_required(VERSION 3.16)
#set(CMAKE_CXX_STANDARD 20)
#
#add_executable(IRSensorTest IRSensorTest.cpp)
#target_link_libraries(IRSensorTest IRSensor)
#
#add_test(NAME TestIRSensorLogic COMMAND IRSensorTest)
#
#target_compile_definitions(IRSensorTest PRIVATE
#        CONFIG_PATH="${CMAKE_SOURCE_DIR}/src/IRSensor/test/config.yaml")
