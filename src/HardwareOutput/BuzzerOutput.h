//
// Created by CANDO on 08/03/2026.
//

#ifndef CROWDLENS_BUZZEROUTPUT_H
#define CROWDLENS_BUZZEROUTPUT_H
#include <memory>
#include <gpiod.hpp>

#include "GPIODigitalOutput.h"
#include "../Common/TrafficEventHandler.h"
#include "../Common/ConfigLoader.h"


namespace gpiod
{
    class line_request;
}

class BUZZEROutput : public GPIODigitalOutput
{
public:
    BUZZEROutput();
    BUZZEROutput(const ConfigLoader& config, const TrafficState& indicationState);
    BUZZEROutput(const ConfigLoader& config, bool skipInit, const TrafficState& indicationState);
    BUZZEROutput(const ConfigLoader& config, int pinNO, const TrafficState& indicationState);
    BUZZEROutput(int pinNO, int chipNO, const TrafficState& indicationState);

    /**
   * Loads configuration settings from the provided ConfigLoader.
   * Updates sensor parameters such as GPIO pin, sensor type, thresholds, and intervals.
   * @param config Configuration loader containing sensor settings to be applied
   */
    void loadConfig(const ConfigLoader& config) override;

    void stop(TrafficState traffic_state) override;

    /**
     * Starts the IR sensor and begins periodic reading based on configured read_interval.
     * Initializes the GPIO pin and starts the timer for traffic detection.
     */
    void run(TrafficState state) override;

    void setBuzzerFrequency(int frequency);
    void setBuzzerBeatsPerCycle(int beatsPerCycle);

private:
    int buzzerFrequency = 2000;
    int buzzerBeatsPerCycle = 4;

protected:
    void worker() override;
};




#endif //CROWDLENS_BUZZEROUTPUT_H
