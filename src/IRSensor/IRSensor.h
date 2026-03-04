//
// Created by CANDO on 21/02/2026.
//

#ifndef CROWDLENS_IRSENSOR_H
#define CROWDLENS_IRSENSOR_H
#include "../Common/ConfigLoader.h"
#include "../Common/TrafficEventHandler.h"
#include <gpiod.hpp>


namespace gpiod
{
    class edge_event;
}

class IRSensor : public TrafficEventHandler
{
public:
    /**
     * Default constructor for IRSensor.
     * Initializes the sensor with default values.
     */
    IRSensor();

    /**
     * Constructor for IRSensor with configuration.
     * @param config Configuration loader containing sensor settings from config file
     */
    explicit IRSensor(const ConfigLoader& config);

    /**
     * Constructor for IRSensor with configuration and initialization control.
     * Allows creation of an IRSensor instance with the option to skip hardware initialization.
     * Useful for testing scenarios where hardware access is not available or desired.
     * @param config Configuration loader containing sensor settings from config file
     * @param skipInit If true, skips hardware initialization (GPIO setup); if false, initializes normally
     */
    explicit IRSensor(const ConfigLoader& config, bool skipInit);

    /**
     * Starts the IR sensor and begins periodic reading based on configured read_interval.
     * Initializes the GPIO pin and starts the timer for traffic detection.
     */
    void run(TrafficState state) override;

    /**
     * Stops the IR sensor timer and cleans up resources.
     * Inherited from TrafficEventHandler.
     */
    void stop(TrafficState state) override;

    /**
     * Loads configuration settings from the provided ConfigLoader.
     * Updates sensor parameters such as GPIO pin, sensor type, thresholds, and intervals.
     * @param config Configuration loader containing sensor settings to be applied
     */
    void loadConfig(const ConfigLoader& config);

private:
    // ::gpiod::chip gpioRequest;
    TrafficState state = TrafficState::NO_TRAFFIC;
    const ConfigLoader& config;
    int lastReadingTime = 0;
    TrafficState lastState = TrafficState::NO_TRAFFIC;
    int maxBlockTime = 100000;
    int GPIOPin = 17;
    int CHIPNo = 0;
    IRSensorType sensorType = IRSensorType::DIGITAL;
    int openThresholdValue = 1;
    int readInterval = 1000;


    // GPIO persistence
    std::shared_ptr<gpiod::chip> chip;
    std::shared_ptr<gpiod::line_request> request;

protected:
    HardwareState sensorState = HardwareState::INIT;

    /**
         * Initializes the IR sensor hardware.
         * Configures the GPIO pin according to sensor type (DIGITAL/ANALOG) and settings.
         */
    virtual void initSensor();

    /**
     * Reads the current value from the IR sensor GPIO pin.
     * Retrieves digital or analog value depending on sensor type configuration.
     */
    virtual void readSensor(gpiod::edge_event e);

    /**
     * Updates the traffic status based on sensor reading.
     * Compares sensor value against threshold and updates state, tracks blocked time.
     */
    virtual void updateState(const TrafficState& newState);

    /**
     * Verifies that the IR sensor hardware is properly initialized and accessible.
     * Checks GPIO chip and line request are valid before sensor operations.
     */
    virtual void verifyHardware();
    /**
    * Reads the actual hardware value.
    * Can be overridden for testing/mocking.
    */
    virtual gpiod::line::value getHardwareValue();

    /**
     * Timer event callback that is called periodically to read the IR sensor.
     * Reads the sensor value and updates the traffic status based on threshold.
     * Inherited from TrafficEventHandler.
     */
    void worker() override;
};


#endif //CROWDLENS_IRSENSOR_H
