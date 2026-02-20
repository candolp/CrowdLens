//
// Created by CANDO on 21/02/2026.
//

#ifndef CROWDLENS_IRSENSOR_H
#define CROWDLENS_IRSENSOR_H
#include "../Common/ConfigLoader.h"
#include "../Common/CppTimer.h"
#include "../Common/TrafficEventHandler.h"


class IRSensor: public TrafficEventHandler
{
    public:
        IRSensor();
        explicit IRSensor(const ConfigLoader& config);
    private:
        traffic_status state = traffic_status::NO_TRAFFIC;
        const ConfigLoader& config;
        int lastStateElaspe = 0;
        int GPIOPin = 0;
        IRSensorType sensorType = IRSensorType::DIGITAL;

    protected:
        void timerEvent() override;
        void stop() override;
};


#endif //CROWDLENS_IRSENSOR_H
