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
    LEDOutput::initHardware();
    _indicationState = indicationState;
}

LEDOutput::LEDOutput(const ConfigLoader& config, int pinNO, const TrafficState& indicationState)
{
    LEDOutput::loadConfig(config);
    GPIOPin = pinNO;
    LEDOutput::initHardware();
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
        traffic_state = state;
        if (workerThread.joinable()) return;
        if (runState != RunState::RUNNING)
        {
            runState = RunState::RUNNING;
            // Initialize sensor hardware

            workerThread = std::thread(&LEDOutput::worker, this);
        }else
        {
            std::cout << "LEDOutput: Already running, ignoring run request" << std::endl;
        }
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
    try
    {
        for(auto & r : eventHandlers)
        {
            r->stop(traffic_state);
        }
        if (request != nullptr && request)
        {
            request->set_value(GPIOPin, gpiod::line::value::INACTIVE);
        }
        if (workerThread.joinable()) workerThread.join();
        std::cout << "LEDOutput: Stopped" << std::endl;
    }catch (const std::exception& e)
    {
        std::cout << "Error:LED output " << e.what() << std::endl;
    }
}

void LEDOutput::initHardware()
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

