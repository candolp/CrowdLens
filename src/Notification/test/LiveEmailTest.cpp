#include "../EmailNotification.h"
#include "../../Common/ConfigLoader.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting Live Email Test..." << std::endl;

    // Load real config
    ConfigLoader config("../../config.yaml");
    
    // Initialize Notifier
    EmailNotification notifier(config);

    // Trigger sending
    std::cout << "Triggering email send via STAMPEDE state..." << std::endl;
    notifier.run(TrafficState::STAMPEDE);

    // Wait for background thread to finish
    std::cout << "Waiting 10 seconds for email process to complete..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Live Email Test Finished." << std::endl;
    return 0;
}
