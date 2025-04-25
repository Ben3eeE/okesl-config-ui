#include "ControlsManager.h"
#include "imgui.h"
#include "keystr.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL.h>

// Map to store control bindings
std::map<std::string, ControlBinding> controls = {
    {"throttle", {"throttle", "up", "*", {"throttle", "spy_up"}, "Throttle"}},
    {"brake", {"brake", "down", "*", {"brake", "spy_down"}, "Brake"}},
    {"brake_alias", {"brake_alias", "", "*", {"brake"}, "Brake alias"}},
    {"ofbrake", {"ofbrake", "q", "+", {"ofbrake"}, "One Frame Brake"}},
    {"left", {"left", "left", "*", {"left", "spy_left"}, "Left Volt"}},
    {"right", {"right", "right", "*", {"right", "spy_right"}, "Right Volt"}},
    {"left;right", {"left;right", "rctrl", "*", {"left", "right"}, "Alovolt"}},
    {"turn", {"turn", "spacebar", "*", {"turn"}, "Turn"}},
    {"save", {"save", "f3", "+", {"save"}, "Save"}},
    {"load", {"load", "f4", "+", {"load", "resetdata"}, "Load"}}
};
// Global list of sections
std::vector<ControlSection> controlSections = {
    {"# Elma Controls", {"throttle", "brake", "brake_alias", "ofbrake", "left", "right", "left;right", "turn"}},
    {"# Saveload Controls", {"save", "load"}}
};

void parseControlLine(const std::string& line, ControlSection& currentSection) {
    std::istringstream iss(line);
    std::string bind, key, actions;

    // Extract the "bind" keyword and the key
    if (!(iss >> bind >> key)) {
        return; // Skip invalid lines
    }

    // Find the position of the first double quote
    size_t quoteStart = line.find('"');
    if (quoteStart == std::string::npos) {
        return; // No actions found
    }

    // Find the position of the second double quote
    size_t quoteEnd = line.find('"', quoteStart + 1);
    if (quoteEnd == std::string::npos) {
        return; // No closing quote found
    }

    // Extract the actions string between the quotes
    actions = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

    // Match the command to the controls map
    for (auto& [name, binding] : controls) {
        // Combine binding.actions into a semicolon-separated string
        std::string combinedActions;
        for (size_t i = 0; i < binding.actions.size(); ++i) {
            combinedActions += binding.actions[i];
            if (i < binding.actions.size() - 1) {
                combinedActions += ";";
            }
        }

        // Compare the combined actions with the actions string
        if (combinedActions == actions) {
            binding.key = key.substr(1);       // Extract the key
            binding.modifiers = key.substr(0, 1); // Extract the modifier
            break;
        }
    }
}

// Function to load controls from controls.cfg
bool loadControls(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse the line
        parseControlLine(line, controlSections[0]); // Pass the first section for simplicity
    }

    file.close();
    return true;
}

// Function to save controls to controls.cfg
bool saveControls(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return false;
    }

    for (const auto& section : controlSections) {
        // Write the section heading
        file << section.heading << std::endl;

        // Write the bindings in this section
        for (const auto& command : section.commands) {
            const auto& binding = controls[command];

            // Combine actions into a single string
            std::string actions;
            for (size_t i = 0; i < binding.actions.size(); ++i) {
                actions += binding.actions[i];
                if (i < binding.actions.size() - 1) {
                    actions += ";";
                }
            }

            // Write the primary binding to the file
            file << "bind " << binding.modifiers << binding.key << " \"" << actions << "\"" << std::endl;

            // Special case: Write an additional line for the "load" key
            if (command == "load" && !binding.key.empty()) {
                file << "bind *" << binding.key << " \"hold\"" << std::endl;
            }
        }

        file << std::endl; // Add a blank line between sections
    }

    file.close();
    return true;
}

// Static variables for keybinding state
static std::string activeBinding = ""; // Command currently being edited
static bool waitingForKey = false;     // Whether we are waiting for a key press

// Function to process keybinding-related events
void processKeybindingEvents(const SDL_Event& event) {
    if (waitingForKey && event.type == SDL_KEYDOWN) {
        int keycode = event.key.keysym.scancode; // Get the SDL scancode
        if (!activeBinding.empty() && controls.find(activeBinding) != controls.end()) {
            controls[activeBinding].key = keystr[keycode]; // Update the keybinding with the key string
            waitingForKey = false;                        // Stop listening for key presses
            activeBinding = "";                           // Clear the active binding
        }
    }
}

// Function to render the controls editor
void renderControlsEditor() {
    ImGui::Begin("Controls Editor");

    ImGui::Text("Press a key to bind it to a command.");
    ImGui::Separator();

    for (const auto& section : controlSections) {
        // Display the section heading
        ImGui::Text("%s", section.heading.c_str());
        ImGui::Separator();

        // Display the bindings in this section
        for (const auto& command : section.commands) {
            auto& binding = controls[command];

            ImGui::Text("%s:", binding.uiName.c_str()); // Use uiName for display
            ImGui::SameLine();

            std::string keyName = binding.key.empty() ? "Unbound" : binding.key;
            if (ImGui::Button(keyName.c_str())) {
                activeBinding = command;
                waitingForKey = true;
            }

            if (waitingForKey && activeBinding == command) {
                ImGui::SameLine();
                ImGui::Text("Press a key...");
            }
        }
    }

    if (ImGui::Button("Save")) {
        saveControls("cfg/controls.cfg");
    }

    ImGui::End();
}