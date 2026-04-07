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
    int GPIOPin = 17;
    int CHIPNo = 0;
    bool available = true;
    int buzzerFrequency = 2000;
    int buzzerBeatsPerCycle = 4;
    //the state for which  the BUZZER will be turn on.
    //this  enables the system to use different BUZZER for different indication
    TrafficState _indicationState = TrafficState::TRAFFIC;
    // GPIO persistence
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> request;

protected:
    void worker() override;
    void virtual initHardware();
};




#endif //CROWDLENS_BUZZEROUTPUT_H
