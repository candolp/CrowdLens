//
// Created by candolp on 20/02/2026.
//

#include <iostream>
#include <string>
#include <stdio.h>

#include "../HardwareOutput/LEDOutput.h"


int main() {
#ifdef CONFIG_PATH
    ConfigLoader config(CONFIG_PATH);
#else
    ConfigLoader config("config.yaml");
#endif

    std::cout << "CrowdLense is Running ..." << std::endl;
    LEDOutput ledOutput  = LEDOutput(17, 0);
    ledOutput.run(TrafficState::TRAFFIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(60000));
    ledOutput.stop(TrafficState::NO_TRAFFIC);
    printf("Press enter\n");
    getchar();
    return 0;
}
