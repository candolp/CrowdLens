//
// Created by CANDO on 08/03/2026.
//

#include "BuzzerOutput.h"
#include <iostream>
#include <string>
#include <format>
#include <gpiod.hpp>



BUZZEROutput::BUZZEROutput()
{
    throw std::runtime_error("BUZZER output requires configuration or pin and chip numbers");
}

BUZZEROutput::BUZZEROutput(const ConfigLoader& config, const TrafficState& indicationState)
{
    BUZZEROutput::loadConfig(config);

    BUZZEROutput::initHardware();
    _indicationState = indicationState;
}

BUZZEROutput::BUZZEROutput(const ConfigLoader& config, bool skipInit, const TrafficState& indicationState)
{
    BUZZEROutput::loadConfig(config);
    if (!skipInit)
    {
        BUZZEROutput::initHardware();
        _indicationState = indicationState;
    }
}

BUZZEROutput::BUZZEROutput(int pinNO, int chipNO, const TrafficState& indicationState)
{
    GPIOPin = pinNO;
    CHIPNo = chipNO;
    available = true;
    BUZZEROutput::initHardware();
    _indicationState = indicationState;
}

BUZZEROutput::BUZZEROutput(const ConfigLoader& config, int pinNO, const TrafficState& indicationState)
{
    BUZZEROutput::loadConfig(config);
    GPIOPin = pinNO;
    BUZZEROutput::initHardware();
    _indicationState = indicationState;
}


void BUZZEROutput::loadConfig(const ConfigLoader& config)
{
    // Load configuration values
    GPIOPin = std::stoi(config.getValue("Hardware_output:BUZZER:pin_number", "27"));
    CHIPNo = std::stoi(config.getValue("infrared_input:chip_number", "0"));
    available = true;
}


void BUZZEROutput::run(const TrafficState state)
{
    //only handle the event if the propagated state matches the expected state for action
    if (_indicationState == state)
    {
        traffic_state = state;
        runState = RunState::RUNNING;
        // Initialize sensor hardware
        workerThread = std::thread(&BUZZEROutput::worker, this);
    }
    else
    {
        //stopping the LED because the dependant traffic state has changed for the current LED indication
        stop(state);
    }
}

void BUZZEROutput::worker()
{
    try
    {
        while (runState == RunState::RUNNING)
        {
            for (int i = 0; i < buzzerBeatsPerCycle; i++)
            {
                // Set GPIO value based on traffic state
                request->set_value(GPIOPin, gpiod::line::value::ACTIVE);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                request->set_value(GPIOPin, gpiod::line::value::INACTIVE);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(buzzerFrequency));
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void BUZZEROutput::setBuzzerFrequency(int frequency)
{
 buzzerFrequency = frequency;
}

void BUZZEROutput::setBuzzerBeatsPerCycle(int beatsPerCycle)
{
    buzzerBeatsPerCycle = beatsPerCycle;
}

inline void BUZZEROutput::stop(TrafficState traffic_state)
{
    runState = RunState::STOPPED;
    for (auto& r : eventHandlers)
    {
        r->stop(traffic_state);
    }
    if (request != nullptr && request)
    {
        request->set_value(GPIOPin, gpiod::line::value::INACTIVE);
    }
    if (workerThread.joinable()) workerThread.join();
}

void BUZZEROutput::initHardware()
{
    // Initialize GPIO chip and line request
    std::cout << "GPIO number is " << GPIOPin << std::endl;
    const std::string chipPath = std::format("/dev/gpiochip{}", CHIPNo);
    const std::string consumername = std::format("gpioconsumer_{}_{}", CHIPNo, GPIOPin);

    try
    {
        // Config the pin as output
        gpiod::line_config line_cfg;
        line_cfg.add_line_settings(
            GPIOPin,
            gpiod::line_settings()
            .set_direction(gpiod::line::direction::OUTPUT));

        chip = std::make_shared<gpiod::chip>(chipPath);

        auto builder = chip->prepare_request();
        builder.set_consumer(consumername);
        builder.set_line_config(line_cfg);
        request = std::make_shared<gpiod::line_request>(builder.do_request());
    }catch (const std::exception& e)
    {
        std::cout << "Error: GPIO initialization failed - "<< CHIPNo << " GPIOid"<< GPIOPin << e.what() << std::endl;
    }
}
