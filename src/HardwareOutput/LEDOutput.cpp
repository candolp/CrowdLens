//
// Created by CANDO on 08/03/2026.
//

#include "LEDOutput.h"
#include <iostream>
#include <string>
#include <format>
#include <gpiod.hpp>

#include "GPIODigitalOutput.h"

LEDOutput::LEDOutput()
{
    throw std::runtime_error("LED output requires configuration or pin and chip numbers");
}

LEDOutput::LEDOutput(const ConfigLoader& config, const TrafficState& indicationState)
{
    LEDOutput::loadConfig(config);
        LEDOutput::initHardware();
        _indicationState = indicationState;

}

LEDOutput::LEDOutput(const ConfigLoader& config, bool skipInit, const TrafficState& indicationState)
{
    LEDOutput::loadConfig(config);
    if (!skipInit)
    {
        LEDOutput::initHardware();
        _indicationState = indicationState;
    }

}

LEDOutput::LEDOutput(int pinNO, int chipNO, const TrafficState& indicationState)
{
    GPIOPin = pinNO;
    CHIPNo = chipNO;
    available = true;
    GPIODigitalOutput::initHardware();
    _indicationState = indicationState;
}

LEDOutput::LEDOutput(const ConfigLoader& config, int pinNO, const TrafficState& indicationState)
{
    LEDOutput::loadConfig(config);
    GPIOPin = pinNO;
    GPIODigitalOutput::initHardware();
    _indicationState = indicationState;
}


void LEDOutput::loadConfig(const ConfigLoader& config)
{
    // Load configuration values
    GPIOPin = std::stoi(config.getValue("Hardware_output:LED:pin_number", "17"));
    CHIPNo = std::stoi(config.getValue("infrared_input:chip_number", "0"));
    available = true;
}




void LEDOutput::run(const TrafficState state)
{
    //only handle the event if the propagated state matches the expected state for action
    if (_indicationState == state)
    {
        if (workerThread.joinable()) return;
        traffic_state = state;
        runState = RunState::RUNNING;
        // Initialize sensor hardware
        workerThread = std::thread(&LEDOutput::worker, this);
    }else
    {
        //stopping the LED because the dependant traffic state has changed for the current LED indication
        stop(state);
    }
}

void LEDOutput::worker()
{
    try
    {
        while (runState == RunState::RUNNING)
        {
            // Set GPIO value based on traffic state
            request->set_value(GPIOPin, gpiod::line::value::ACTIVE);
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            request->set_value(GPIOPin, gpiod::line::value::INACTIVE);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
}


void LEDOutput::stop(TrafficState traffic_state) {
    runState = RunState::STOPPED;
    for(auto & r : eventHandlers)
    {
        r->stop(traffic_state);
    }
    if (request != nullptr && request)
    {
        request->set_value(GPIOPin, gpiod::line::value::INACTIVE);
    }
    if (workerThread.joinable()) workerThread.join();
}


