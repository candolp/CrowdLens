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

#include "../../../lib/gpiodcxx/line.hpp"

// Mock class to simulate hardware input and observe behavior
class TestableIRSensor : public IRSensor {
public:
    using IRSensor::IRSensor; // Inherit constructors

    gpiod::line::value simulatedValue = gpiod::line::value::INACTIVE;

    // Override to simulate hardware without needing libgpiod access
    gpiod::line::value getHardwareValue() override {
        return simulatedValue;
    }

    // Expose internal logic for testing without timer complexity
    void tick() {
        timerEvent();
    }
};

// Mock Runnable to verify if the sensor triggers events
class MockRunnable : public Runnable {
public:
    bool runCalled = false;
    bool stopCalled = false;

    void run() override { runCalled = true; }
    void stop() override { stopCalled = true; }

    void reset() { runCalled = false; stopCalled = false; }
};

void testIRSensorLogic() {
    ConfigLoader config; // Assumes default config is accessible or empty mock
    TestableIRSensor sensor(config);
    MockRunnable mock;

    sensor.registerEventRunnable(mock);

    // Scenario 1: Traffic detected (Value >= Threshold)
    sensor.simulatedValue = gpiod::line::value::ACTIVE;

    // We need to 'tick' it enough times to exceed maxBlockTime (default 10000ms)
    // with a readInterval (default 500ms or 1000ms)
    for(int i = 0; i < 25; ++i) {
        sensor.tick();
    }

    assert(mock.runCalled && "Runnable::run should be called when traffic is detected for long enough");
    mock.reset();

    // Scenario 2: Traffic cleared (Value < Threshold)
    sensor.simulatedValue = gpiod::line::value::INACTIVE;
    for(int i = 0; i < 25; ++i) {
        sensor.tick();
    }

    assert(mock.stopCalled && "Runnable::stop should be called when traffic is cleared");

    std::cout << "IRSensor Tests Passed!" << std::endl;
}

int main() {
    try {
        testIRSensorLogic();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}