//
// Created by CANDO on 21/02/2026.
//

#ifndef CROWDLENS_CONFIGLOADER_H
#define CROWDLENS_CONFIGLOADER_H

#include <string>
#include <map>
#include <vector>

class ConfigLoader
{
public:
    // Default constructor - loads from default location
    ConfigLoader();

    // Constructor with custom filename
    explicit ConfigLoader(const std::string& filename);

    // Loads the YAML file into memory
    bool load(const std::string& filename);

    // Retrieves a value using a colon-separated path, e.g., "AI:model"
    std::string getValue(const std::string& path, const std::string& defaultValue = "") const;

private:
    std::map<std::string, std::string> configData;

    // Helper to trim whitespace
    static std::string trim(const std::string& s);
};

#endif //CROWDLENS_CONFIGLOADER_H