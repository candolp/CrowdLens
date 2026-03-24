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

    GPIODigitalOutput::initHardware();
    _indicationState = indicationState;
}

BUZZEROutput::BUZZEROutput(const ConfigLoader& config, bool skipInit, const TrafficState& indicationState)
{
    BUZZEROutput::loadConfig(config);
    if (!skipInit)
    {
        GPIODigitalOutput::initHardware();
        _indicationState = indicationState;
    }
}

BUZZEROutput::BUZZEROutput(int pinNO, int chipNO, const TrafficState& indicationState)
{
    GPIOPin = pinNO;
    CHIPNo = chipNO;
    available = true;
    GPIODigitalOutput::initHardware();
    _indicationState = indicationState;
}

BUZZEROutput::BUZZEROutput(const ConfigLoader& config, int pinNO, const TrafficState& indicationState)
{
    BUZZEROutput::loadConfig(config);
    GPIOPin = pinNO;
    GPIODigitalOutput::initHardware();
    _indicationState = indicationState;
}


void BUZZEROutput::loadConfig(const ConfigLoader& config)
{
    // Load configuration values
    GPIOPin = std::stoi(config.getValue("Hardware_output:BUZZER:pin_number", "17"));
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
