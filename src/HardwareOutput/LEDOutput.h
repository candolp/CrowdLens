//
// Created by CANDO on 08/03/2026.
//

#ifndef CROWDLENS_LEDOUTPUT_H
#define CROWDLENS_LEDOUTPUT_H
#include <memory>
#include <gpiod.hpp>

#include "GPIODigitalOutput.h"
#include "../Common/TrafficEventHandler.h"
#include "../Common/ConfigLoader.h"


namespace gpiod
{
    class line_request;
}

class LEDOutput : public GPIODigitalOutput
{
public:
    LEDOutput();
    LEDOutput(const ConfigLoader& config, const TrafficState& indicationState);
    LEDOutput(const ConfigLoader& config, bool skipInit, const TrafficState& indicationState);
    LEDOutput(const ConfigLoader& config, int pinNO, const TrafficState& indicationState);
    LEDOutput( int pinNO, int chipNO, const TrafficState& indicationState);

    /**
   * Loads configuration settings from the provided ConfigLoader.
   * Updates sensor parameters such as GPIO pin, sensor type, thresholds, and intervals.
   * @param config Configuration loader containing sensor settings to be applied
   */
    void loadConfig(const ConfigLoader& config) override;

    /**
     * Starts the IR sensor and begins periodic reading based on configured read_interval.
     * Initializes the GPIO pin and starts the timer for traffic detection.
     */
    void run(TrafficState state) override;

    void stop(TrafficState traffic_state) override;

protected:
    void worker() override;

};


#endif //CROWDLENS_LEDOUTPUT_H
