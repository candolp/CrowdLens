//
// Created by CANDO on 23/03/2026.
//

#include "../BuzzerOutput.h"
#include <cassert>
#include <iostream>

class TestableBuzzerOutput : public BUZZEROutput
{
public:
    TestableBuzzerOutput(const ConfigLoader& config, const TrafficState& indicationState)
        : BUZZEROutput(config,true, indicationState)
    {
    }

protected:
    void initHardware() override
    {
        // Skip real GPIO initialization in tests
    }

    void worker() override
    {
        // Skip real hardware loop in tests
    }
};

int main()
{
    try
    {
        ConfigLoader config;
        TestableBuzzerOutput buzzer(config, TrafficState::TRAFFIC);

        // Matching state should attempt to start the output logic
        buzzer.run(TrafficState::TRAFFIC);

        // Non-matching state should be handled without throwing
        buzzer.run(TrafficState::NO_TRAFFIC);

        buzzer.setBuzzerFrequency(1500);
        buzzer.setBuzzerBeatsPerCycle(6);

        std::cout << "BuzzerOutput test passed.\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "BuzzerOutput test failed: " << e.what() << '\n';
        return 1;
    }
}