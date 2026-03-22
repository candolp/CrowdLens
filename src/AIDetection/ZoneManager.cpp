#include "ZoneManager.h"

namespace cl {

void ZoneManager::addZone(Zone zone) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string name = zone.name();
    zones_.insert_or_assign(std::move(name), std::move(zone));
}

void ZoneManager::removeZone(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    zones_.erase(name);
}

std::vector<Zone> ZoneManager::getZones() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Zone> result;
    for (const auto& [name, zone] : zones_) {
        result.push_back(zone);
    }
    return result;
}

std::optional<Zone> ZoneManager::findZone(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::map<std::string, Zone>::const_iterator it = zones_.find(name);
    return it != zones_.end() ? std::optional<Zone>{it->second} : std::nullopt;
}

}
