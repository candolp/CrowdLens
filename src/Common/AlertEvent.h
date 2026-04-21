#pragma once

#include "../Common/CrowdLensState.h"
#include "../Common/CrowdMetrics.h"

#include <string>

namespace cl {

enum class AlertType {
    NO_ALERT, // all zones below threshold, system is idle
    // reactive: fires when the condition is already present
    CONGESTION, // density has crossed densityThreshold_
    CHOKEPOINT, // density high + flow already low
    FLOW_REVERSAL,
    // predictive: fires when the trend will reach the threshold within horizon
    STAMPEDE_RISK, // density rising fast enough to hit stampede threshold within horizon
    CHOKEPOINT_PREDICTED // density rising and flow falling; both will converge to chokepoint within horizon
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
