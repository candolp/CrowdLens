//
// Created by CANDO on 27/02/2026.
//

#ifndef CROWDLENS_CROWLENSESTATE_H
#define CROWDLENS_CROWLENSESTATE_H

enum TrafficState {
    NO_TRAFFIC,
    TRAFFIC,
    CROWDED,
    STAMPEDE
};
enum IRSensorType
{
    DIGITAL,
    ANALOG
};
enum RunState
{
    RUNNING,
    STOPPED,
    PAUSED,
    STARTING,
    STOPPING
};

enum HardwareState
{
    INIT,
    READY,
    ERROR
};

#endif //CROWDLENS_CROWLENSESTATE_H