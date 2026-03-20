//
// Created by candolp on 20/02/2026.
//

#include <iostream>
#include <string>
#include <stdio.h>

#include "../HardwareOutput/LEDOutput.h"
#include "../IRSensor/IRSensor.h"


int main() {
#ifdef CONFIG_PATH
    ConfigLoader config(CONFIG_PATH);
#else
    ConfigLoader config("config.yaml");
#endif

    std::cout << "CrowdLense is Running ..." << std::endl;
    LEDOutput greenLED  = LEDOutput(22, 0, TrafficState::TRAFFIC);
    LEDOutput redLED  = LEDOutput(10, 0,TrafficState::STAMPEDE);
    IRSensor isensor(config);
    // isensor.registerEventRunnable(greenLED);
    // isensor.registerEventRunnable(redLED);
    // isensor.run(TrafficState::NO_TRAFFIC);
    redLED.run(TrafficState::STAMPEDE);
    greenLED.run(TrafficState::TRAFFIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    printf("Press enter\n");
    getchar();
    greenLED.stop(TrafficState::NO_TRAFFIC);
    redLED.stop(TrafficState::NO_TRAFFIC);
    isensor.stop(TrafficState::NO_TRAFFIC);
    return 0;
}
