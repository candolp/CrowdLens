//
// Created by CANDO on 08/03/2026.
//

#include "LEDOutput.h"
#include <iostream>
#include <string>
#include <format>
#include <gpiod.hpp>

LEDOutput::LEDOutput()
{
    throw std::runtime_error("LED output requires configuration or pin and chip numbers");
}

LEDOutput::LEDOutput(const ConfigLoader& config, bool skipInit)
{loadConfig(config);
    if (!skipInit)
    {
        LEDOutput::initHardware();
    }

}

LEDOutput::LEDOutput(int pinNO, int chipNO)
{
    GPIOPin = pinNO;
    CHIPNo = chipNO;
    available = true;
    LEDOutput::initHardware();
}

LEDOutput::LEDOutput(const ConfigLoader& config, int pinNO)
{
    loadConfig(config);
    GPIOPin = pinNO;
    LEDOutput::initHardware();
}


void LEDOutput::loadConfig(const ConfigLoader& config)
{
    // Load configuration values
    GPIOPin = std::stoi(config.getValue("Hardware_output:LED:pin_number", "17"));
    CHIPNo = std::stoi(config.getValue("infrared_input:chip_number", "0"));
    available = true;
}


void LEDOutput::initHardware()
{
    // Initialize GPIO chip and line request
    const std::string chipPath = std::format("/dev/gpiochip{}", CHIPNo);
    const std::string consumername = std::format("gpioconsumer_{}_{}", CHIPNo, GPIOPin);

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



