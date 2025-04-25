#ifndef CONTROLSMANAGER_H
#define CONTROLSMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <SDL.h>

// Structure to store control bindings
struct ControlBinding {
    std::string command;  // Command name
    std::string key;      // Key string (e.g., "+f4")
    std::string modifiers; // Modifiers (e.g., "+", "*", "-")
    std::vector<std::string> actions; // Actions bound to the key
    std::string uiName;
};

// Structure to store a section of controls
struct ControlSection {
    std::string heading; // Section heading (e.g., "#elma controls")
    std::vector<std::string> commands; // Commands in this section
};

// Global map to store control bindings
extern std::map<std::string, ControlBinding> controls;

// List of sections to preserve order and headings
extern std::vector<ControlSection> controlSections;

// Function declarations
bool loadControls(const std::string& filename);
bool saveControls(const std::string& filename);
void renderControlsEditor();
void processKeybindingEvents(const SDL_Event& event);

#endif // CONTROLSMANAGER_H