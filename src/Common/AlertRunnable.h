#pragma once

#include "Runnable.h"
#include "AlertEvent.h"

namespace cl {

// extends Runnable for subscribers that need the full alert payload
class AlertRunnable : public Runnable {
public:
    void run(TrafficState /*state*/) override {}
    virtual void onAlert(const AlertEvent& event) = 0;
    virtual ~AlertRunnable() = default;
};

}
