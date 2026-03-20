//
// Created by CANDO on 08/03/2026.
//

#ifndef CROWDLENS_LEDOUTPUT_H
#define CROWDLENS_LEDOUTPUT_H
#include <memory>
#include <gpiod.hpp>

#include "../Common/TrafficEventHandler.h"
#include "../Common/ConfigLoader.h"


namespace gpiod
{
    class line_request;
}

class LEDOutput : public TrafficEventHandler
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
    void loadConfig(const ConfigLoader& config);

    /**
     * Starts the IR sensor and begins periodic reading based on configured read_interval.
     * Initializes the GPIO pin and starts the timer for traffic detection.
     */
    void run(TrafficState state) override;

private:
    int GPIOPin = 17;
    int CHIPNo = 0;
    bool available = false;
    //the state for which  the LED will be turn on.
    //this  enables the system to use different LED for different indication
    TrafficState _indicationState = TrafficState::TRAFFIC;
    // GPIO persistence
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> request;

protected:
    void worker() override;
    virtual void initHardware();
};


#endif //CROWDLENS_LEDOUTPUT_H
