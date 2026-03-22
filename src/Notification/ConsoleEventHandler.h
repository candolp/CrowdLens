#pragma once

#include "../Common/AlertRunnable.h"

namespace cl {

// prints alert details to stdout
class ConsoleEventHandler : public AlertRunnable {
public:
    void onAlert(const AlertEvent& event) override;
    void run(TrafficState /*state*/) override {}
    void stop(TrafficState /*state*/) override {}
};

}
