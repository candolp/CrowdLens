#include "ConfigLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::unordered_map<std::string, std::string>
loadConfig(const std::string& filename)
{
    std::unordered_map<std::string, std::string> config;

    std::ifstream file(filename);

    if (!file.is_open())
        throw std::runtime_error("Could not open config file: " + filename);

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string key, value;

        if (std::getline(iss, key, '=') && std::getline(iss, value))
        {
            config[key] = value;
        }
    }

    return config;
}