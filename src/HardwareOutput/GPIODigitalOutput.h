//
// Created by CANDO on 08/03/2026.
//

#ifndef CROWDLENS_GPIODigitalOUTPUT_H
#define CROWDLENS_GPIODigitalOUTPUT_H
#include <memory>
#include <gpiod.hpp>

#include "../Common/TrafficEventHandler.h"
#include "../Common/ConfigLoader.h"


namespace gpiod
{
    class line_request;
}

class GPIODigitalOutput : public TrafficEventHandler
{
public:
    /**
   * Loads configuration settings from the provided ConfigLoader.
   * Updates sensor parameters such as GPIO pin, sensor type, thresholds, and intervals.
   * @param config Configuration loader containing sensor settings to be applied
   */
    virtual void loadConfig(const ConfigLoader& config);



protected:
    int GPIOPin = 17;
    int CHIPNo = 0;
    bool available = false;
    // the state for which this output will be activated
    TrafficState _indicationState = TrafficState::TRAFFIC;
    // GPIO persistence
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> request;

    virtual void initHardware();
};


#endif //CROWDLENS_GPIODigitalOUTPUT_H
