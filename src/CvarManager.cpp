#include "CvarManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "imgui.h"

using json = nlohmann::json;

// Global map to store cvars
std::map<std::string, Cvar> cvars;

// Function to load cvars from a .cfg file
void loadCvarsFromConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << " for reading. Using default values." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        std::string value;
        if (!(iss >> key >> value)) {
            continue; // Skip invalid lines
        }

        auto it = cvars.find(key);
        if (it != cvars.end()) {
            Cvar& cvar = it->second;
            if (cvar.type == "bool") {
                cvar.boolValue = (value == "1");
            } else if (cvar.type == "int") {
                cvar.intValue = std::stoi(value);
            } else if (cvar.type == "float") {
                cvar.floatValue = std::stof(value);
            } else if (cvar.type == "color") {
                unsigned int color = std::stoul(value, nullptr, 16);
                cvar.colorValue[0] = ((color >> 24) & 0xFF) / 255.0f; // Red
                cvar.colorValue[1] = ((color >> 16) & 0xFF) / 255.0f; // Green
                cvar.colorValue[2] = ((color >> 8) & 0xFF) / 255.0f;  // Blue
                cvar.colorValue[3] = (color & 0xFF) / 255.0f;         // Alpha
            }
        }
    }

    file.close();
    std::cout << "Configuration loaded from " << filename << std::endl;
}

// Function to load cvars from JSON and config files
bool loadCvars(const std::string& jsonFilename, const std::string& configFilename) {
    // Load min/max and default values from JSON
    std::ifstream jsonFile(jsonFilename);
    if (!jsonFile.is_open()) {
        std::cerr << "Failed to open " << jsonFilename << std::endl;
        return false;
    }

    json j;
    jsonFile >> j;

    for (auto& [key, value] : j.items()) {
        Cvar cvar;
        cvar.name = key;
        cvar.type = value["type"];
        if (cvar.type == "bool") {
            cvar.boolValue = value["default"];
        } else if (cvar.type == "int") {
            cvar.intValue = value["default"];
            cvar.minValue = value["min"];
            cvar.maxValue = value["max"];
        } else if (cvar.type == "float") {
            cvar.floatValue = value["default"];
            cvar.minValue = value["min"];
            cvar.maxValue = value["max"];
        } else if (cvar.type == "color") {
            std::string colorHex = value["default"];
            if (colorHex[0] == '#') colorHex = colorHex.substr(1); // Remove '#'
            unsigned int color = std::stoul(colorHex, nullptr, 16);
            cvar.colorValue[0] = ((color >> 24) & 0xFF) / 255.0f; // Red
            cvar.colorValue[1] = ((color >> 16) & 0xFF) / 255.0f; // Green
            cvar.colorValue[2] = ((color >> 8) & 0xFF) / 255.0f;  // Blue
            cvar.colorValue[3] = (color & 0xFF) / 255.0f;         // Alpha
        }
        cvars[key] = cvar;
    }

    // Load actual values from config file
    loadCvarsFromConfig(configFilename);

    return true;
}

// Function to save cvars to a .cfg file
void saveCvarsToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    for (const auto& [key, cvar] : cvars) {
        file << std::left << std::setw(20) << key; // Align the key to 20 characters
        if (cvar.type == "bool") {
            file << (cvar.boolValue ? "1" : "0");
        } else if (cvar.type == "int") {
            file << cvar.intValue;
        } else if (cvar.type == "float") {
            file << std::fixed << std::setprecision(3) << cvar.floatValue; // Save float with 3 decimal places
        } else if (cvar.type == "color") {
            unsigned int color = 
                ((unsigned int)(cvar.colorValue[0] * 255) << 24) | // Red
                ((unsigned int)(cvar.colorValue[1] * 255) << 16) | // Green
                ((unsigned int)(cvar.colorValue[2] * 255) << 8)  | // Blue
                ((unsigned int)(cvar.colorValue[3] * 255));        // Alpha
            file << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << color;

            // Reset stream manipulators after writing hex
            file << std::dec << std::setfill(' ') << std::setw(0);
        }
        file << std::endl;
    }

    file.close();
    std::cout << "Configuration saved to " << filename << std::endl;
}

// Function to render the cvars GUI
void renderCvars() {
    // Create a vector of cvars sorted by type
    std::vector<std::pair<std::string, Cvar*>> sortedCvars;
    for (auto& [key, cvar] : cvars) {
        sortedCvars.emplace_back(key, &cvar);
    }

    // Sort the cvars by type (alphabetically)
    std::sort(sortedCvars.begin(), sortedCvars.end(), [](const std::pair<std::string, Cvar*>& a, const std::pair<std::string, Cvar*>& b) {
        return a.second->type < b.second->type; // Compare the type strings
    });

    // Render the sorted cvars
    for (auto& [key, cvarPtr] : sortedCvars) {
        Cvar& cvar = *cvarPtr; // Dereference the pointer to access the actual Cvar object
        ImGui::PushID(key.c_str()); // Use the cvar name as a unique ID

        if (cvar.type == "bool") {
            ImGui::Checkbox("##bool", &cvar.boolValue); // Render the control
        } else if (cvar.type == "int") {
            ImGui::SliderInt("##int", &cvar.intValue, (int)cvar.minValue, (int)cvar.maxValue);
        } else if (cvar.type == "float") {
            ImGui::SliderFloat("##float", &cvar.floatValue, cvar.minValue, cvar.maxValue);
        } else if (cvar.type == "color") {
            ImGui::ColorEdit4("##color", cvar.colorValue);
        }

        ImGui::SameLine(); // Place the label on the same line as the control
        ImGui::Text("%s", key.c_str()); // Render the label

        ImGui::PopID(); // Restore the previous ID
    }

    if (ImGui::Button("Save")) {
        saveCvarsToFile("cfg/cvars.cfg");
    }
}
