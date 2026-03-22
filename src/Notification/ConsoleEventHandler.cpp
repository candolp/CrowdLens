#include "ConsoleEventHandler.h"

#include <iostream>

namespace cl {

void ConsoleEventHandler::onAlert(const AlertEvent& event) {
    std::cout << "[ALERT] " << event.message
              << " | zone: " << event.metrics.zoneName
              << " | density: " << event.metrics.density
              << "\n";
}

}
