#pragma once

#include "Zone.h"

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace cl {

class ZoneManager {
public:
    ZoneManager() = default;

    void addZone(Zone zone);

    void removeZone(const std::string& name);

    std::vector<Zone> getZones() const;

    std::optional<Zone> findZone(const std::string& name) const;

private:
    mutable std::mutex mutex_;
    std::map<std::string, Zone> zones_;
};

}
