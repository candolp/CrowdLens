//
// Created by CANDO on 21/02/2026.
//

#include "ConfigLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <filesystem>

ConfigLoader::ConfigLoader()
{
    load("config.yaml");
}

ConfigLoader::ConfigLoader(const std::string& filename)
{
    load(filename);
}

std::string ConfigLoader::trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

bool ConfigLoader::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cout << "file not found: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::vector<std::pair<int, std::string>> stack; // Stores {indent_level, key_name}

    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || trim(line)[0] == '#') continue;

        int indent = 0;
        while (indent < line.length() && line[indent] == ' ') indent++;

        std::string content = trim(line);
        size_t colonPos = content.find(':');
        if (colonPos == std::string::npos) continue;

        std::string key = trim(content.substr(0, colonPos));
        std::string value = trim(content.substr(colonPos + 1));

        // Adjust stack based on indentation
        while (!stack.empty() && stack.back().first >= indent) {
            stack.pop_back();
        }
        stack.push_back({indent, key});

        if (!value.empty()) {
            std::string fullKey;
            for (size_t i = 0; i < stack.size(); ++i) {
                fullKey += stack[i].second + (i == stack.size() - 1 ? "" : ":");
            }
            configData[fullKey] = value;
        }
    }
    return true;
}

std::string ConfigLoader::getValue(const std::string& path, const std::string& defaultValue) const {
    auto it = configData.find(path);
    if (it != configData.end()) {
        return it->second;
    }
    return defaultValue;
}