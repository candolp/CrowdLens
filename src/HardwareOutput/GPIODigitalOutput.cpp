//
// Created by CANDO on 08/03/2026.
//


#include <iostream>
#include <string>
#include <format>
#include <gpiod.hpp>

#include "GPIODigitalOutput.h"

void GPIODigitalOutput::initHardware()
{
    // Initialize GPIO chip and line request
    std::cout << "GPIO number is " << CHIPNo << std::endl;
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

void GPIODigitalOutput::loadConfig(const ConfigLoader& config){
    // Load configuration values
    GPIOPin = std::stoi(config.getValue("Hardware_output:pin_number", "17"));
    CHIPNo = std::stoi(config.getValue("infrared_input:chip_number", "0"));
    available = true;
};

