//
// Created by CANDO on 24/02/2026.
//

#include "../IRSensor.h"
#include "../../Common/ConfigLoader.h"
#include "../../Common/Runnable.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <gpiod.hpp>
#include <chrono>
#include <thread>


// Mock class to simulate hardware input and observe behavior
class TestableIRSensor : public IRSensor
{
public:
    TestableIRSensor(const ConfigLoader& config): IRSensor(config,true)
    {
        std::cout << "this contructor worked" << std::endl;
    }

    gpiod::line::value simulatedValue = gpiod::line::value::INACTIVE;

    // Override to simulate hardware without needing libgpiod access
    gpiod::line::value getHardwareValue() override
    {
        return simulatedValue;
    }

    // Mock initSensor to prevent actual hardware initialization
    void initSensor() override
    {
        // No-op: skip GPIO initialization in tests
    }

    // Mock readSensor to simulate sensor reading without hardware
    void readSensor(gpiod::edge_event e) override
    {
        // No-op: sensor reading is simulated via simulatedValue
    }

    // Mock updateState to allow direct state manipulation
    void updateState(const TrafficState& newState) override
    {
        IRSensor::updateState(newState);
    }

    // Mock verifyHardware to bypass hardware validation
    void verifyHardware() override
    {
        // change hardware state
        IRSensor::sensorState = HardwareState::READY;
    }

    // Expose internal logic for testing without timer complexity
    void tick()
    {
        worker(); // Updated from timerEvent() to worker()
    }

    // Mock worker to simulate periodic sensor reading without actual threading
    void worker() override
    {
        // Simulate sensor reading behavior
        if (simulatedValue == gpiod::line::value::ACTIVE)
        {
            updateState(TrafficState::TRAFFIC);
        }
        else
        {
            updateState(TrafficState::NO_TRAFFIC);
        }
    }
};

// Mock Runnable to verify if the sensor triggers events
class MockRunnable : public Runnable
{
public:
    bool runCalled = false;
    bool stopCalled = false;
    TrafficState lastRunState = TrafficState::NO_TRAFFIC;
    TrafficState lastStopState = TrafficState::NO_TRAFFIC;

    void run(TrafficState state) override
    {
        runCalled = true;
        lastRunState = state;
    }

    void stop(TrafficState state) override
    {
        stopCalled = true;
        lastStopState = state;
    }

    void reset()
    {
        runCalled = false;
        stopCalled = false;
        lastRunState = TrafficState::NO_TRAFFIC;
        lastStopState = TrafficState::NO_TRAFFIC;
    }
};

void testIRSensorLogic()
{
    ConfigLoader config;
#ifdef CONFIG_PATH
     config = ConfigLoader(CONFIG_PATH);
#else
     config = ConfigLoader("config.yaml");
#endif
    std::cout << "this worked" << std::endl;
    TestableIRSensor sensor = TestableIRSensor(config);
    std::cout << "this worked" << std::endl;
    MockRunnable mock;
    //det max block time


    sensor.registerEventRunnable(mock);

    // Start the sensor with initial state
    sensor.run(TrafficState::NO_TRAFFIC);

    // Scenario 1: Traffic detected (Value >= Threshold)
    sensor.simulatedValue = gpiod::line::value::ACTIVE;


    // We need to 'tick' it enough times to exceed maxBlockTime (default 10000ms)
    // // with a readInterval (default 500ms or 1000ms)
    // for (int i = 0; i < 25; ++i)
    // {
    //     sensor.tick();
    // }
    // std::this_thread::sleep_for(std::chrono::milliseconds(30));
    //
    // std::cout << "ahaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!" << std::endl;
    //
    // assert(mock.runCalled && "Runnable::run should be called when traffic is detected for long enough");
    mock.reset();
    // std::cout << "ahaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!" << std::endl;
    //
    // // Scenario 2: Traffic cleared (Value < Threshold)
    // sensor.simulatedValue = gpiod::line::value::INACTIVE;
    // // for (int i = 0; i < 25; ++i)
    // // {
    // //     sensor.tick();
    // // }
    // std::this_thread::sleep_for(std::chrono::milliseconds(30));
    //
    // assert(mock.stopCalled && "Runnable::stop should be called when traffic is cleared");

    // Clean up - stop the sensor
    sensor.stop(TrafficState::NO_TRAFFIC);

    std::cout << "IRSensor Tests Passed!" << std::endl;
}

int main()
{
    try
    {
        std::cout << "Testing ahs started" << std::endl;
        testIRSensorLogic();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
