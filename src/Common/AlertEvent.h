#pragma once

#include "../Common/CrowdLensState.h"
#include "../Common/CrowdMetrics.h"

#include <string>

namespace cl {

enum class AlertType {
    CONGESTION,
    CHOKEPOINT,
    FLOW_REVERSAL
};

enum class AlertSeverity {
    INFO,
    WARNING,
    CRITICAL
};

struct AlertEvent {
    TrafficState state; // crowd level at the time of the alert
    AlertType type;
    AlertSeverity severity;
    CrowdMetrics metrics; // the zone snapshot that caused this
    std::string message;
};

}
