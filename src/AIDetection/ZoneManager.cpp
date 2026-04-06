#include "ZoneManager.h"
#include "../Common/ConfigLoader.h"
#include <sstream>

namespace cl {

static ZoneType zoneTypeFromString(const std::string& s) {
    if (s == "ENTRANCE") return ZoneType::ENTRANCE;
    if (s == "EXIT") return ZoneType::EXIT;
    if (s == "CHOKEPOINT") return ZoneType::CHOKEPOINT;
    return ZoneType::GENERAL;
}

ZoneManager::ZoneManager(const ConfigLoader& config) {
    loadConfig(config);
}

void ZoneManager::loadConfig(const ConfigLoader& config) {
    std::string namesStr = config.getValue("zones:names", "");
    std::istringstream namesStream(namesStr);
    std::string zoneName;
    while (std::getline(namesStream, zoneName, ',')) {
        size_t start = zoneName.find_first_not_of(" \t");
        size_t end = zoneName.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        zoneName = zoneName.substr(start, end - start + 1);

        std::string type = config.getValue("zones:zone." + zoneName + ":type", "GENERAL");
        std::string pointsStr = config.getValue("zones:zone." + zoneName + ":points", "");
        std::istringstream pointsStream(pointsStr);
        std::string token;
        std::vector<cv::Point> poly;
        while (pointsStream >> token) {
            size_t comma = token.find(',');
            if (comma != std::string::npos)
                poly.emplace_back(std::stoi(token.substr(0, comma)), std::stoi(token.substr(comma + 1)));
        }
        Zone zone(zoneName, poly, zoneTypeFromString(type));

        std::string prefix = "zones:zone." + zoneName + ":";
        std::string densityStr = config.getValue(prefix + "density_threshold", "");
        std::string chokepointStr = config.getValue(prefix + "chokepoint_threshold", "");
        std::string flowStr = config.getValue(prefix + "flow_magnitude_threshold", "");
        std::string pppStr = config.getValue(prefix + "pixels_per_person", "");

        if (!densityStr.empty()) zone.setDensityThreshold(std::stof(densityStr));
        if (!chokepointStr.empty()) zone.setChokepointThreshold(std::stof(chokepointStr));
        if (!flowStr.empty()) zone.setFlowMagnitudeThreshold(std::stof(flowStr));
        if (!pppStr.empty()) zone.setPixelsPerPerson(std::stoi(pppStr));

        addZone(std::move(zone));
    }
}

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
