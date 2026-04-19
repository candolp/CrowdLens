// //
// // Created by CANDO on 21/02/2026.
// //
//
// #include "IRSensor.h"
// #include <chrono>
// #include <thread>
// #include <iostream>
// #include <string>
// #include <format>
// #include <gpiod.hpp>
//
//
// IRSensor::IRSensor() : config(*(ConfigLoader*)nullptr)
// {
//     throw std::runtime_error("IRSensor requires configuration");
// }
//
// IRSensor::IRSensor(const ConfigLoader& config) : config(config)
// {
//     IRSensor::loadConfig(config);
//     IRSensor::initSensor();
// }
//
// IRSensor::IRSensor(const ConfigLoader& config, bool skipInit) : config(config)
// {
//     IRSensor::loadConfig(config);
//     if (!skipInit)
//     {
//         IRSensor::initSensor();
//     }
// }
//
// void IRSensor::loadConfig(const ConfigLoader& config)
// {
//     // Load configuration values
//     GPIOPin = std::stoi(config.getValue("infrared_input:pin_number", "22"));
//     CHIPNo = std::stoi(config.getValue("infrared_input:chip_number", "0"));
//     readInterval = std::stoi(config.getValue("infrared_input:read_interval", "500"));
//     maxBlockTime = std::stoi(config.getValue("infrared_input:max_blocked_time", "10000"));
//     openThresholdValue = std::stoi(config.getValue("infrared_input:open_threshold_value", "1"));
//     std::string sensorTypeStr = config.getValue("infrared_input:type", "DIGITAL");
//     std::cout << "Running rubbish constructor !!!!!!!!!!!!!!!!!!!" << std::endl;
//     if (sensorTypeStr == "ANALOG")
//     {
//         sensorType = IRSensorType::ANALOG;
//     }
//     else
//     {
//         sensorType = IRSensorType::DIGITAL;
//     }
// }
//
// void IRSensor::initSensor()
// {
//     // Initialize GPIO chip and line request
//     const std::string chipPath = std::format("/dev/gpiochip{}", CHIPNo);
//     const std::string consumername = std::format("gpioconsumer_{}_{}", CHIPNo, GPIOPin);
//     // const std::string chipPath = "/dev/gpiochip" + std::to_string(CHIPNo);
//     // const std::string consumername = "gpioconsumer_"+ std::to_string(CHIPNo) + "_" + std::to_string(GPIOPin);
//     std::cout << "chip path: " << chipPath << " consumer: " << consumername  << std::endl;
//
//     // Config the pin as input and detecting falling and rising edegs
//     gpiod::line_config line_cfg;
//     line_cfg.add_line_settings(
//         GPIOPin,
//         gpiod::line_settings()
//         .set_direction(gpiod::line::direction::INPUT)
//         .set_edge_detection(gpiod::line::edge::BOTH));
//     std::cout << "this is redicolous" << std::endl;
//     chip = std::make_shared<gpiod::chip>(chipPath);
//     std::cout << "this is redicolous !  !!!!!!!!!!!!!!" << std::endl;
//     auto builder = chip->prepare_request();
//     builder.set_consumer(consumername);
//     builder.set_line_config(line_cfg);
//     request = std::make_shared<gpiod::line_request>(builder.do_request());
//     std::cout << "this is redicolous1111111111111111111111111111" << std::endl;
//     verifyHardware();
//     std::cout << "this is redicolous 222222222222222222222" << std::endl;
// }
//
// void IRSensor::readSensor(gpiod::edge_event e)
// {
//     // Determine traffic state based on sensor reading
//     TrafficState newState;
//     if (sensorType == IRSensorType::DIGITAL)
//     {
//         if (gpiod::edge_event::event_type::RISING_EDGE == e.type())
//         {
//             newState = TrafficState::NO_TRAFFIC;
//         }
//         else
//         {
//             newState = TrafficState::TRAFFIC;
//         }
//     }
//     else
//     {
//         // For analog, treat value as integer comparison
//         // Read GPIO value
//         gpiod::line::value value = getHardwareValue();
//         auto now = std::chrono::system_clock::now();
//         auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
//         std::cout << "Sensor reading. time: " << timestamp << " value: " << static_cast<int>(value) << std::endl;
//         newState = (static_cast<int>(value) >= openThresholdValue) ? TrafficState::TRAFFIC : TrafficState::NO_TRAFFIC;
//     }
//
//     updateState(newState);
// }
//
// void IRSensor::updateState(const TrafficState& newState)
// {
//     int timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(
//         std::chrono::system_clock::now().time_since_epoch()).count();
//     int elapsedTime = timeNow - lastReadingTime;
//     // Check if blocked time exceeds maximum
//     if (newState == TrafficState::TRAFFIC && elapsedTime >= maxBlockTime)
//     {
//         traffic_state = TrafficState::TRAFFIC;
//         eventCallback(traffic_state);
//     }
//     else if (newState == TrafficState::NO_TRAFFIC && elapsedTime > maxBlockTime)
//     {
//         traffic_state = TrafficState::NO_TRAFFIC;
//         //update dependent event
//         eventCallback(traffic_state);
//     }
//
//     if (newState != lastState)
//     {
//         lastReadingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
//             std::chrono::system_clock::now().time_since_epoch()).count();;
//         lastState = newState;
//     }
// }
//
// gpiod::line::value IRSensor::getHardwareValue()
// {
//     return request->get_value(GPIOPin);
// }
//
// void IRSensor::run(TrafficState state)
// {
//     this->state = state;
//     runState = RunState::RUNNING;
//     // Initialize sensor hardware
//     workerThread = std::thread(&IRSensor::worker, this);
// }
//
// void IRSensor::verifyHardware()
// {
//     try
//     {
//         gpiod::line::value value = request->get_value(GPIOPin);
//         if (value != gpiod::line::value::ACTIVE)
//         {
//             std::cout << "GPIO " << GPIOPin << " is connected to the sensor" << std::endl;
//             sensorState = HardwareState::READY;
//         }
//         else
//         {
//             std::cout << "GPIO " << GPIOPin << " is  connected to the sensor and the value is LOW" << std::endl;
//             sensorState = HardwareState::READY;
//         }
//     }catch (const std::exception& e)
//     {
//         std::cout << "Error: " << e.what() << std::endl;
//         std::cout << "GPIO " << GPIOPin << " is not connected to the sensor" << std::endl;
//         sensorState = HardwareState::ERROR;
//     }
// }
//
// void IRSensor::worker()
// {
//     while (runState == RunState::RUNNING)
//     {
//         try
//         {
//             bool res = request->wait_edge_events(std::chrono::milliseconds(readInterval));
//             if (res)
//             {
//                 gpiod::edge_event_buffer buffer;
//                 request->read_edge_events(buffer, 1);
//                 readSensor(buffer.get_event(0));
//             }
//             else
//             {
//                 verifyHardware();
//                 if (sensorState != HardwareState::READY)
//                 {
//                     std::cout << "The sensor is not responding. Please check " << std::endl;
//                 }
//                 else
//                 {
//                     updateState(lastState);
//                 }
//             }
//         }
//         catch (const std::exception& e)
//         {
//             std::cout << "Error: " << e.what() << std::endl;
//         }
//     }
// }
//
//
// void IRSensor::stop(TrafficState state)
// {
//     // Clean up GPIO resources
//     if (request)
//     {
//         request.reset();
//     }
//     if (chip)
//     {
//         chip.reset();
//     }
//
//     // Stop the worker thread and notify handlers
//     TrafficEventHandler::stop(state);
// }
