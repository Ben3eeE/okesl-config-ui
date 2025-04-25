#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <SDL.h>
#include <algorithm> // For std::sort

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

using json = nlohmann::json;

// Structure to hold cvar data
struct Cvar {
    std::string type;
    std::string name;
    union {
        bool boolValue;
        int intValue;
        float floatValue;
    };
    float minValue;
    float maxValue;
    float colorValue[4]; // RGBA values for color cvars
    std::string stringValue;
};

// Global map to store cvars
std::map<std::string, Cvar> cvars;

// Function to load cvars from JSON
bool loadCvars(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << std::endl;
        return false;
    }

    json j;
    file >> j;

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
            // Parse color from hex string (e.g., "#RRGGBBAA")
            std::string colorHex = value["default"];
            if (colorHex[0] == '#') colorHex = colorHex.substr(1); // Remove '#'
            unsigned int color = std::stoul(colorHex, nullptr, 16);
            cvar.colorValue[0] = ((color >> 24) & 0xFF) / 255.0f; // Red
            cvar.colorValue[1] = ((color >> 16) & 0xFF) / 255.0f; // Green
            cvar.colorValue[2] = ((color >> 8) & 0xFF) / 255.0f;  // Blue
            cvar.colorValue[3] = (color & 0xFF) / 255.0f;         // Alpha
        } else if (cvar.type == "string") {
            cvar.stringValue = value["default"];
        }
        cvars[key] = cvar;
    }

    return true;
}

// Function to render the cvars GUI
void renderCvars() {
    // Create a vector of cvars sorted by type
    std::vector<std::pair<std::string, Cvar>> sortedCvars(cvars.begin(), cvars.end());
    std::sort(sortedCvars.begin(), sortedCvars.end(), [](const auto& a, const auto& b) {
        return a.second.type < b.second.type; // Sort by type
    });

    // Render the sorted cvars
    for (auto& [key, cvar] : sortedCvars) {
        ImGui::PushID(key.c_str()); // Use the cvar name as a unique ID
        ImGui::Text("%s", key.c_str()); // Display the name once as a label

        if (cvar.type == "bool") {
            ImGui::Checkbox("##bool", &cvar.boolValue); // Use a hidden label with "##"
        } else if (cvar.type == "int") {
            ImGui::SliderInt("##int", &cvar.intValue, (int)cvar.minValue, (int)cvar.maxValue);
        } else if (cvar.type == "float") {
            ImGui::SliderFloat("##float", &cvar.floatValue, cvar.minValue, cvar.maxValue);
        } else if (cvar.type == "color") {
            ImGui::ColorEdit4("##color", cvar.colorValue);
        }

        ImGui::PopID(); // Restore the previous ID
    }
}

// Main code
int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Cvars Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return -1;
    }
    //SDL_RendererInfo info;
    //SDL_GetRendererInfo(renderer, &info);
    //SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Load cvars
    if (!loadCvars("assets/cvars.json")) {
        return -1;
    }

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Render cvars GUI
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver); // Set default window size
        ImGui::Begin("Cvars Editor");
        renderCvars();
        ImGui::End();

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}