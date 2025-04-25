#ifndef CVARMANAGER_H
#define CVARMANAGER_H

#include <string>
#include <map>
#include <nlohmann/json.hpp>

// Structure to hold cvar data
struct Cvar {
    std::string type;
    std::string name;
    bool boolValue = false;
    int intValue = 0;
    float floatValue = 0.0f;
    float minValue = 0.0f;
    float maxValue = 0.0f;
    float colorValue[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // RGBA values for color cvars
};

// Global map to store cvars
extern std::map<std::string, Cvar> cvars;

// Function declarations
void loadCvarsFromConfig(const std::string& filename);
bool loadCvars(const std::string& jsonFilename, const std::string& configFilename);
void saveCvarsToFile(const std::string& filename);
void renderCvars();

#endif // CVARMANAGER_H