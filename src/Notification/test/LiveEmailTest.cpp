//
// Created by Amantle on 20/03/2026.
//

#include "../EmailNotification.h"
#include "../../Common/ConfigLoader.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting Live Email Test..." << std::endl;

    // CONFIG_PATH is injected by CMake and points to build/config.yaml
    // Fall back to searching next to the binary if the macro isn't set.
#ifdef CONFIG_PATH
    const std::string configPath = CONFIG_PATH;
#else
    const std::string configPath = "config.yaml";
#endif

    std::cout << "Loading config from: " << configPath << std::endl;
    ConfigLoader config(configPath);

    // Initialize Notifier
    EmailNotification notifier(config, TrafficState::STAMPEDE);

    // Trigger STAMPEDE alert — run() spawns the worker thread
    std::cout << "Triggering email send via STAMPEDE state..." << std::endl;
    notifier.run(TrafficState::STAMPEDE);

    // Give the worker thread time to finish sending before we stop
    std::cout << "Waiting for email process to complete..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(15));

    // Explicitly stop (joins the worker thread)
    notifier.stop(TrafficState::NO_TRAFFIC);

    std::cout << "Live Email Test Finished." << std::endl;
    return 0;
}
