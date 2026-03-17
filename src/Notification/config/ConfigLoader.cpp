#include "ConfigLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

// Trim leading and trailing whitespace from a string.
static std::string trim(const std::string& s)
{
    auto start = std::find_if_not(s.begin(), s.end(),
                                  [](unsigned char c){ return std::isspace(c); });
    auto end   = std::find_if_not(s.rbegin(), s.rend(),
                                  [](unsigned char c){ return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : std::string{};
}

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
        // Skip blank lines and comments
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty() || trimmedLine[0] == '#')
            continue;

        // Split only on the FIRST '=' so values containing '=' (e.g. base64) are preserved
        auto eqPos = trimmedLine.find('=');
        if (eqPos == std::string::npos)
            continue;  // malformed line — skip silently

        std::string key   = trim(trimmedLine.substr(0, eqPos));
        std::string value = trim(trimmedLine.substr(eqPos + 1));

        if (!key.empty())
            config[key] = value;
    }

    return config;
}