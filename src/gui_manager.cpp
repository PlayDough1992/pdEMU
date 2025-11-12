#include "gui_manager.h"
#include "system_database.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/backends/imgui_impl_sdl2.h"
#include "../../external/imgui/backends/imgui_impl_sdlrenderer2.h"
#include <algorithm>
#include <iostream>
#include <cstdio>  // for rename()

GuiManager::GuiManager()
    : m_initialized(false)
    , m_imguiContext(nullptr)
    , m_selectedRomIndex(-1)
    , m_showAbout(false)
{
    memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
}

GuiManager::~GuiManager() {
    shutdown();
}

bool GuiManager::init(SDL_Window* window, SDL_Renderer* renderer) {
    if (m_initialized) {
        return true;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    m_imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext((ImGuiContext*)m_imguiContext);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();
    
    // Customize colors for a nice retro gaming look
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 0.95f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.3f, 0.5f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.4f, 0.6f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.3f, 0.4f, 0.6f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.5f, 0.7f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.6f, 0.8f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.3f, 0.4f, 0.6f, 0.8f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.4f, 0.5f, 0.7f, 0.8f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.6f, 0.8f, 0.8f);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    m_initialized = true;
    return true;
}

void GuiManager::shutdown() {
    if (m_initialized) {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        if (m_imguiContext) {
            ImGui::DestroyContext((ImGuiContext*)m_imguiContext);
            m_imguiContext = nullptr;
        }
        m_initialized = false;
    }
}

void GuiManager::beginFrame() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void GuiManager::endFrame(SDL_Renderer* renderer) {
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

void GuiManager::processEvent(SDL_Event* event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

bool GuiManager::wantsCaptureMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

bool GuiManager::wantsCaptureKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

void GuiManager::renderMainMenu(bool& showMenu, bool& showSettings) {
    if (!m_initialized || !showMenu) return;

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Main Menu", &showMenu, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("pdEMU");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Resume Game", ImVec2(-1, 40))) {
            showMenu = false;
        }

        ImGui::Spacing();

        if (ImGui::Button("Settings", ImVec2(-1, 40))) {
            showSettings = true;
            showMenu = false;
        }

        ImGui::Spacing();

        if (ImGui::Button("Quit", ImVec2(-1, 40))) {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
        }

        ImGui::End();
    }
}

void GuiManager::renderRomBrowser(RomManager& romManager, std::string& selectedRom, bool& shouldLaunch, bool& showSettings) {
    if (!m_initialized) return;
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("ROM Browser", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Select a ROM to launch");
    ImGui::Separator();
    
    // Search filter
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("Search", m_searchBuffer, sizeof(m_searchBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
    }
    
    ImGui::Separator();
    
    // Get ROMs organized by system
    auto romsBySystem = romManager.getRomsBySystem();
    
    // Create columns for better layout
    ImGui::BeginChild("RomListChild", ImVec2(0, -40), true);
    
    std::string searchStr = m_searchBuffer;
    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);
    
    // Display ROMs grouped by system
    for (const auto& systemPair : romsBySystem) {
        const std::string& systemName = systemPair.first;
        const std::vector<RomInfo>& roms = systemPair.second;

        if (roms.empty()) continue;

        // Get system display name from first ROM
        std::string systemDisplayName = roms[0].systemDisplayName;

        // Create collapsible header for each system
        std::string headerLabel = systemDisplayName + " (" + std::to_string(roms.size()) + " games)";
        if (ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();

            for (size_t i = 0; i < roms.size(); ++i) {
                const auto& rom = roms[i];
                
                // Apply search filter
                if (searchStr.length() > 0) {
                    std::string romName = rom.displayName;
                    std::transform(romName.begin(), romName.end(), romName.begin(), ::tolower);
                    if (romName.find(searchStr) == std::string::npos) {
                        continue;
                    }
                }
                
                // Display ROM entry
                ImGui::PushID(rom.fullPath.c_str());
                
                bool isSelected = (rom.fullPath == selectedRom);
                

                // ROM name as selectable (takes most of the width)
                ImGui::BeginGroup();
                if (ImGui::Selectable(rom.displayName.c_str(), isSelected, 0, ImVec2(ImGui::GetContentRegionAvail().x - 120, 20))) {
                    selectedRom = rom.fullPath;
                    m_selectedRomIndex = i;
                    // If this is a Dolphin ROM, remember it for backend selection
                    if (rom.systemName == "gamecube" || rom.systemName == "wii") {
                        setLastDolphinRom(rom.fullPath);
                    }
                }

                // Dolphin backend selector (show only for selected Dolphin ROM)
                if (isSelected && (rom.systemName == "gamecube" || rom.systemName == "wii")) {
                    const char* backends[] = { "OpenGL", "Vulkan" };
                    ImGui::Text("Dolphin Video Backend:");
                    ImGui::SameLine();
                    ImGui::Combo("##dolphin_backend", &m_dolphinBackend, backends, IM_ARRAYSIZE(backends));
                }

                // Double-click to launch
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    selectedRom = rom.fullPath;
                    shouldLaunch = true;
                    // If this is a Dolphin ROM, remember it for backend selection
                    if (rom.systemName == "gamecube" || rom.systemName == "wii") {
                        setLastDolphinRom(rom.fullPath);
                    }
                }

                // Show file size on hover
                if (ImGui::IsItemHovered()) {
                    float sizeMB = rom.fileSize / (1024.0f * 1024.0f);
                    ImGui::SetTooltip("%s\nSize: %.2f MB\nFile: %s", 
                        rom.displayName.c_str(), sizeMB, rom.filename.c_str());
                }
                ImGui::EndGroup();
                
                // "Change System" button on the same line
                ImGui::SameLine();
                if (ImGui::SmallButton("Change System")) {
                    ImGui::OpenPopup("system_selector");
                    selectedRom = rom.fullPath;  // Remember which ROM we're changing
                }
                
                // System selection popup
                if (ImGui::BeginPopup("system_selector")) {
                    ImGui::Text("Select System for:");
                    ImGui::Text("%s", rom.displayName.c_str());
                    ImGui::Separator();
                    
                    auto& systemDb = *romManager.getSystemDatabase();
                    for (const auto& systemPair : systemDb.getAllSystems()) {
                        const SystemInfo& system = systemPair.second;
                        if (ImGui::Selectable(system.displayName.c_str())) {
                            // Rename the file to include the system marker
                            std::string oldPath = rom.fullPath;
                            std::string dir = oldPath.substr(0, oldPath.find_last_of('/'));
                            std::string filename = rom.filename;
                            
                            // Remove any existing marker
                            for (const auto& otherSys : systemDb.getAllSystems()) {
                                const std::string& marker = otherSys.second.marker;
                                if (!marker.empty()) {
                                    size_t markerPos = filename.find(marker);
                                    if (markerPos != std::string::npos) {
                                        filename.erase(markerPos, marker.length());
                                        break;
                                    }
                                }
                            }
                            
                            // Add new marker before the FIRST period (not last)
                            // This handles multi-part extensions like .nkit.iso
                            size_t dotPos = filename.find('.');
                            if (dotPos != std::string::npos && !system.marker.empty()) {
                                filename.insert(dotPos, system.marker);
                            }
                            
                            std::string newPath = dir + "/" + filename;
                            
                            // Rename the file
                            if (rename(oldPath.c_str(), newPath.c_str()) == 0) {
                                std::cout << "Renamed: " << oldPath << " -> " << newPath << std::endl;
                                // Rescan directory to update ROM list
                                romManager.scanDirectory("ROMS");
                            } else {
                                std::cerr << "Failed to rename file" << std::endl;
                            }
                            
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    
                    ImGui::EndPopup();
                }
                
                ImGui::PopID();
            }
            
            ImGui::Unindent();
        }
    }
    
    ImGui::EndChild();
    
    // Debug mode toggle (for all systems, but especially useful for OpenGL hardware rendering)
    ImGui::Separator();
    ImGui::Checkbox("Debug Mode (Skip/Log OpenGL Errors)", &m_debugMode);
    ImGui::SameLine();
    ImGui::TextDisabled("?");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("When enabled, the emulator will catch and log OpenGL errors, skipping invalid calls so the game can continue for debugging.");
    }
    ImGui::Separator();
    
    if (!selectedRom.empty()) {
        if (ImGui::Button("Launch Game", ImVec2(120, 30))) {
            shouldLaunch = true;
        }
        ImGui::SameLine();
    }
    
    if (ImGui::Button("Refresh List", ImVec2(120, 30))) {
        romManager.scanDirectory("ROMS");
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Settings", ImVec2(120, 30))) {
        showSettings = true;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Exit", ImVec2(120, 30))) {
        shouldLaunch = false;
        selectedRom = "";
    }

    ImGui::End();
}

void GuiManager::renderSettings(EmulatorConfig& config, bool& shouldApply) {
    if (!m_initialized) return;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse)) {
        
        if (ImGui::CollapsingHeader("Video Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderInt("Window Scale", &config.windowScale, 1, 6);
            ImGui::SliderInt("Internal Resolution Scale", &config.internalScale, 1, 4);
            ImGui::Checkbox("VSync", &config.vsync);
            ImGui::Checkbox("Linear Filtering", &config.linearFilter);
            ImGui::Checkbox("Show FPS", &config.showFPS);
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Internal scale increases rendering quality");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Audio Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Volume", &config.audioVolume, 0.0f, 1.0f, "%.2f");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Controls Info")) {
            ImGui::TextWrapped(
                "Arrow Keys: D-Pad\n"
                "Z: A Button\n"
                "X: B Button\n"
                "A: L Trigger\n"
                "S: R Trigger\n"
                "Enter: Start\n"
                "Shift: Select\n"
                "Tab: Fast Forward\n"
                "F1: Toggle FPS\n"
                "F2/F3: Change internal scale\n"
                "F4: Toggle filter\n"
                "ESC: Menu/Quit"
            );
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::Button("Apply & Close", ImVec2(-1, 35))) {
            shouldApply = true;
        }

        ImGui::End();
    }
}

void GuiManager::renderFPSCounter(float fps) {
    if (!m_initialized) return;
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    ImGui::Text("FPS: %.1f", fps);
    ImGui::End();
}
