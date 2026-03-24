//
// Created by CANDO on 23/03/2026.
//

#include "../LEDOutput.h"
#include <cassert>
#include <iostream>

class TestableLEDOutput : public LEDOutput
{
public:
    TestableLEDOutput(const ConfigLoader& config, const TrafficState& indicationState)
        : LEDOutput(config, true, indicationState)
    {
    }

    void initHardware() override
    {
        // Skip real GPIO initialization in tests
        std::cout << "initHardware called !!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    }

    void worker() override
    {
        // Skip real hardware loop in tests
        std::cout << "worker called !!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    }
};

int main()
{
    try
    {
        ConfigLoader config;
        TestableLEDOutput led(config, TrafficState::TRAFFIC);

        // Should start when the matching state is provided
        led.run(TrafficState::TRAFFIC);

        // Should stop gracefully when a different state is provided
        led.run(TrafficState::NO_TRAFFIC);

        std::cout << "LEDOutput test passed.\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "LEDOutput test failed: " << e.what() << '\n';
        return 1;
    }
}