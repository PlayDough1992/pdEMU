#include "gui_manager.h"
#include "system_database.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/backends/imgui_impl_sdl2.h"
#include "../../external/imgui/backends/imgui_impl_sdlrenderer2.h"
#include <algorithm>
#include <iostream>
#include <cstdio>  // for rename()
#include <cstring> // for strncpy()

GuiManager::GuiManager()
    : m_initialized(false)
    , m_imguiContext(nullptr)
    , m_selectedRomIndex(-1)
    , m_showAbout(false)
    , m_lastNavTime(0.0f)
    , m_navRepeatDelay(0.15f)
    , m_totalVisibleRoms(0)
    , m_focusMode(UIFocusMode::RomList)
    , m_buttonFocusIndex(0)
    , m_showOnScreenKeyboard(false)
    , m_selectedButtonIndex(0)
    , m_shouldQuit(false)
    , m_settingsItemIndex(0)
    , m_settingsTotalItems(0)
    , m_keyboardRow(0)
    , m_keyboardCol(0)
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
    
    // Get the current display size from ImGui IO
    ImGuiIO& io = ImGui::GetIO();
    
    // Make the ROM browser window fill the entire screen with padding
    float padding = 20.0f;
    ImGui::SetNextWindowPos(ImVec2(padding, padding), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - padding * 2, io.DisplaySize.y - padding * 2), ImGuiCond_Always);
    
    if (!ImGui::Begin("pdEMU - ROM Browser", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Select a ROM to launch");
    ImGui::Separator();
    
    // Show controller navigation hint
    const char* focusModeText = "";
    switch (m_focusMode) {
        case UIFocusMode::RomList:
            focusModeText = "[ROM List] Use L1/R1 to switch modes";
            break;
        case UIFocusMode::SearchBox:
            focusModeText = "[Search Box] Press X to open keyboard | L1/R1 to switch modes";
            break;
        case UIFocusMode::Buttons:
            focusModeText = "[Buttons] Use Left/Right to select, X to activate | L1/R1 to switch modes";
            break;
        default:
            break;
    }
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", focusModeText);
    ImGui::Separator();
    
    // Search filter - highlight if focused
    if (m_focusMode == UIFocusMode::SearchBox) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.5f, 0.3f, 0.5f));
    }
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("Search", m_searchBuffer, sizeof(m_searchBuffer));
    if (m_focusMode == UIFocusMode::SearchBox) {
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
    }
    
    ImGui::Separator();
    
    // Get ROMs organized by system
    auto romsBySystem = romManager.getRomsBySystem();
    
    // Calculate available height for the child window (subtract header, search bar, separators, and bottom padding)
    float availableHeight = ImGui::GetContentRegionAvail().y - 50;
    
    // Create scrollable child window for ROM list
    ImGui::BeginChild("RomListChild", ImVec2(0, availableHeight), true);
    
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
                

                // ROM name as selectable (leave space for button + checkbox on the right)
                ImGui::BeginGroup();
                if (ImGui::Selectable(rom.displayName.c_str(), isSelected, 0, ImVec2(ImGui::GetContentRegionAvail().x - 240, 20))) {
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
                
                // Debug mode checkbox on the same line
                ImGui::SameLine();
                ImGui::Checkbox("Debug", &m_debugMode);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Enable debug mode to catch and log OpenGL errors");
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
                            
                            // Use platform-appropriate path separator
                            #ifdef _WIN32
                            char sep = '\\';
                            #else
                            char sep = '/';
                            #endif
                            
                            std::string dir = oldPath.substr(0, oldPath.find_last_of("/\\"));
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
                            
                            std::string newPath = dir + sep + filename;
                            
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
    
    // Bottom buttons
    ImGui::Separator();
    
    int buttonIndex = 0;
    
    if (!selectedRom.empty()) {
        bool isSelected = (m_focusMode == UIFocusMode::Buttons && m_selectedButtonIndex == buttonIndex);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        }
        
        if (ImGui::Button("Launch Game", ImVec2(120, 30))) {
            shouldLaunch = true;
        }
        
        if (isSelected) {
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
        buttonIndex++;
    }
    
    bool isRefreshSelected = (m_focusMode == UIFocusMode::Buttons && m_selectedButtonIndex == buttonIndex);
    if (isRefreshSelected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    }
    
    if (ImGui::Button("Refresh List", ImVec2(120, 30))) {
        romManager.scanDirectory("ROMS");
    }
    
    if (isRefreshSelected) {
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    buttonIndex++;
    
    bool isSettingsSelected = (m_focusMode == UIFocusMode::Buttons && m_selectedButtonIndex == buttonIndex);
    if (isSettingsSelected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    }
    
    if (ImGui::Button("Settings", ImVec2(120, 30))) {
        showSettings = true;
    }
    
    if (isSettingsSelected) {
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    buttonIndex++;
    
    bool isExitSelected = (m_focusMode == UIFocusMode::Buttons && m_selectedButtonIndex == buttonIndex);
    if (isExitSelected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    }
    
    if (ImGui::Button("Exit", ImVec2(120, 30))) {
        shouldLaunch = false;
        selectedRom = "";
        m_shouldQuit = true;
    }
    
    if (isExitSelected) {
        ImGui::PopStyleColor();
    }

    ImGui::End();
    
    // Render on-screen keyboard if active (navigation is handled in handleControllerNavigation)
    if (m_showOnScreenKeyboard) {
        bool shouldClose = false;
        bool shouldApply = false;
        renderOnScreenKeyboard(shouldClose, shouldApply);
    }
}

void GuiManager::renderSettings(EmulatorConfig& config, bool& shouldApply) {
    if (!m_initialized) return;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse)) {
        
        int itemIndex = 0;
        
        if (ImGui::CollapsingHeader("Video Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Window Scale
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.5f, 0.3f, 0.5f));
            }
            ImGui::SliderInt("Window Scale", &config.windowScale, 1, 6);
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PopStyleColor();
            }
            itemIndex++;
            
            // Internal Resolution Scale
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.5f, 0.3f, 0.5f));
            }
            ImGui::SliderInt("Internal Resolution Scale", &config.internalScale, 1, 4);
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PopStyleColor();
            }
            itemIndex++;
            
            // VSync
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.5f, 0.3f, 0.5f));
            }
            ImGui::Checkbox("VSync", &config.vsync);
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PopStyleColor();
            }
            itemIndex++;
            
            // Linear Filtering
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.5f, 0.3f, 0.5f));
            }
            ImGui::Checkbox("Linear Filtering", &config.linearFilter);
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PopStyleColor();
            }
            itemIndex++;
            
            // Show FPS
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.5f, 0.3f, 0.5f));
            }
            ImGui::Checkbox("Show FPS", &config.showFPS);
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PopStyleColor();
            }
            itemIndex++;
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Internal scale increases rendering quality");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Audio Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Volume
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.5f, 0.3f, 0.5f));
            }
            ImGui::SliderFloat("Volume", &config.audioVolume, 0.0f, 1.0f, "%.2f");
            if (m_settingsItemIndex == itemIndex) {
                ImGui::PopStyleColor();
            }
            itemIndex++;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Controls Info")) {
            ImGui::TextWrapped(
                "Controller Navigation:\n"
                "D-Pad/Stick: Navigate settings\n"
                "Left/Right: Adjust values\n"
                "X: Toggle checkboxes\n"
                "Circle: Apply & Close\n"
                "\n"
                "Keyboard:\n"
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

        // Apply & Close button
        if (m_settingsItemIndex == itemIndex) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        }
        if (ImGui::Button("Apply & Close", ImVec2(-1, 35))) {
            shouldApply = true;
        }
        if (m_settingsItemIndex == itemIndex) {
            ImGui::PopStyleColor();
        }
        itemIndex++;
        
        m_settingsTotalItems = itemIndex;

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

void GuiManager::renderOnScreenKeyboard(bool& shouldClose, bool& shouldApply) {
    if (!m_initialized) return;
    
    ImGuiIO& io = ImGui::GetIO();
    
    // Center the keyboard on screen
    float keyboardWidth = 900.0f;
    float keyboardHeight = 500.0f;
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - keyboardWidth) * 0.5f, (io.DisplaySize.y - keyboardHeight) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(keyboardWidth, keyboardHeight), ImGuiCond_Always);
    
    ImGui::Begin("On-Screen Keyboard", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    // Display current input
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##search_input", const_cast<char*>(m_keyboardInput.c_str()), m_keyboardInput.capacity() + 1, ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(2);
    
    ImGui::Spacing();
    ImGui::Text("Use D-Pad/Stick to navigate, X to select, Circle to backspace");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Keyboard layout grid
    const char* keys[][13] = {
        {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", nullptr},
        {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", nullptr},
        {"A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", nullptr, nullptr},
        {"Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", nullptr, nullptr, nullptr},
        {"SPACE", "BACKSPACE", "CLEAR", "CANCEL", "SEARCH", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
    };
    
    const int rowSizes[] = {12, 12, 11, 10, 5};
    const int numRows = 5;
    
    float buttonWidth = 60.0f;
    float buttonHeight = 55.0f;
    float spacing = 4.0f;
    
    // Clamp keyboard position
    if (m_keyboardRow < 0) m_keyboardRow = 0;
    if (m_keyboardRow >= numRows) m_keyboardRow = numRows - 1;
    if (m_keyboardCol < 0) m_keyboardCol = 0;
    if (m_keyboardCol >= rowSizes[m_keyboardRow]) m_keyboardCol = rowSizes[m_keyboardRow] - 1;
    
    // Render keyboard grid
    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < rowSizes[row]; col++) {
            if (keys[row][col] == nullptr) break;
            
            // Calculate button width for special keys
            float btnWidth = buttonWidth;
            if (strcmp(keys[row][col], "SPACE") == 0) {
                btnWidth = buttonWidth * 3.0f;
            } else if (strcmp(keys[row][col], "BACKSPACE") == 0 || strcmp(keys[row][col], "SEARCH") == 0) {
                btnWidth = buttonWidth * 1.8f;
            } else if (strcmp(keys[row][col], "CLEAR") == 0 || strcmp(keys[row][col], "CANCEL") == 0) {
                btnWidth = buttonWidth * 1.3f;
            }
            
            // Highlight selected key
            bool isSelected = (m_keyboardRow == row && m_keyboardCol == col);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
            }
            
            // Render button
            ImGui::Button(keys[row][col], ImVec2(btnWidth, buttonHeight));
            
            if (isSelected) {
                ImGui::PopStyleColor(3);
            }
            
            // Add spacing between buttons
            if (col < rowSizes[row] - 1 && keys[row][col + 1] != nullptr) {
                ImGui::SameLine(0, spacing);
            }
        }
    }
    
    ImGui::End();
}

void GuiManager::handleControllerNavigation(ControllerProfileManager& controllerManager, RomManager& romManager, std::string& selectedRom, bool& shouldLaunch, bool& showSettings) {
    if (!m_initialized) return;
    
    // Get the active controller (first one with input)
    int instanceId = controllerManager.getActiveController();
    if (instanceId < 0) return;
    
    auto navState = controllerManager.getUINavState(instanceId);
    
    // Get current time for input repeat
    float currentTime = SDL_GetTicks() / 1000.0f;
    bool canNavigate = (currentTime - m_lastNavTime) > m_navRepeatDelay;
    
    if (!canNavigate) return;
    
    // Handle on-screen keyboard navigation
    if (m_showOnScreenKeyboard) {
        // Keyboard layout dimensions
        const int rowSizes[] = {12, 12, 11, 10, 5};
        const char* keys[][13] = {
            {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", nullptr},
            {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", nullptr},
            {"A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", nullptr, nullptr},
            {"Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", nullptr, nullptr, nullptr},
            {"SPACE", "BACKSPACE", "CLEAR", "CANCEL", "SEARCH", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
        };
        
        bool navigated = false;
        
        // Navigate keyboard grid
        if (navState.upPressed) {
            if (m_keyboardRow > 0) {
                m_keyboardRow--;
                // Clamp column to row size
                if (m_keyboardCol >= rowSizes[m_keyboardRow]) {
                    m_keyboardCol = rowSizes[m_keyboardRow] - 1;
                }
                navigated = true;
            }
        } else if (navState.downPressed) {
            if (m_keyboardRow < 4) {
                m_keyboardRow++;
                // Clamp column to row size
                if (m_keyboardCol >= rowSizes[m_keyboardRow]) {
                    m_keyboardCol = rowSizes[m_keyboardRow] - 1;
                }
                navigated = true;
            }
        } else if (navState.leftPressed) {
            if (m_keyboardCol > 0) {
                m_keyboardCol--;
                navigated = true;
            }
        } else if (navState.rightPressed) {
            if (m_keyboardCol < rowSizes[m_keyboardRow] - 1) {
                m_keyboardCol++;
                navigated = true;
            }
        }
        
        if (navigated) {
            m_lastNavTime = currentTime;
        }
        
        // Handle key selection
        if (navState.confirmPressed) {
            const char* selectedKey = keys[m_keyboardRow][m_keyboardCol];
            
            if (strcmp(selectedKey, "SPACE") == 0) {
                m_keyboardInput += ' ';
            } else if (strcmp(selectedKey, "BACKSPACE") == 0) {
                if (!m_keyboardInput.empty()) {
                    m_keyboardInput.pop_back();
                }
            } else if (strcmp(selectedKey, "CLEAR") == 0) {
                m_keyboardInput.clear();
            } else if (strcmp(selectedKey, "CANCEL") == 0) {
                m_showOnScreenKeyboard = false;
                m_keyboardInput.clear();
                m_focusMode = UIFocusMode::SearchBox;
            } else if (strcmp(selectedKey, "SEARCH") == 0) {
                // Apply search
                strncpy(m_searchBuffer, m_keyboardInput.c_str(), sizeof(m_searchBuffer) - 1);
                m_searchBuffer[sizeof(m_searchBuffer) - 1] = '\0';
                m_showOnScreenKeyboard = false;
                m_keyboardInput.clear();
                m_focusMode = UIFocusMode::RomList;
            } else {
                // Regular character key
                m_keyboardInput += selectedKey[0];
            }
            
            m_lastNavTime = currentTime;
        } else if (navState.cancelPressed) {
            // Circle button = backspace
            if (!m_keyboardInput.empty()) {
                m_keyboardInput.pop_back();
            }
            m_lastNavTime = currentTime;
        }
        
        return;
    }
    
    // Handle focus mode switching with shoulder buttons
    if (navState.shoulderLeftPressed) {
        // L1/LB - cycle focus mode backward
        switch (m_focusMode) {
            case UIFocusMode::RomList:
                m_focusMode = UIFocusMode::Buttons;
                break;
            case UIFocusMode::SearchBox:
                m_focusMode = UIFocusMode::RomList;
                break;
            case UIFocusMode::Buttons:
                m_focusMode = UIFocusMode::SearchBox;
                break;
            default:
                break;
        }
        m_lastNavTime = currentTime;
        return;
    } else if (navState.shoulderRightPressed) {
        // R1/RB - cycle focus mode forward
        switch (m_focusMode) {
            case UIFocusMode::RomList:
                m_focusMode = UIFocusMode::SearchBox;
                break;
            case UIFocusMode::SearchBox:
                m_focusMode = UIFocusMode::Buttons;
                break;
            case UIFocusMode::Buttons:
                m_focusMode = UIFocusMode::RomList;
                break;
            default:
                break;
        }
        m_lastNavTime = currentTime;
        return;
    }
    
    // Handle navigation based on current focus mode
    switch (m_focusMode) {
        case UIFocusMode::SearchBox:
            // When focused on search box, confirm opens keyboard
            if (navState.confirmPressed) {
                m_showOnScreenKeyboard = true;
                m_keyboardInput = m_searchBuffer;
                m_keyboardRow = 0;
                m_keyboardCol = 0;
                m_focusMode = UIFocusMode::OnScreenKeyboard;
                m_lastNavTime = currentTime;
            } else if (navState.cancelPressed) {
                // Cancel clears search
                memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
                m_lastNavTime = currentTime;
            }
            break;
            
        case UIFocusMode::RomList:
            // Original ROM list navigation
            {
                // Get ROMs organized by system for counting
                auto romsBySystem = romManager.getRomsBySystem();
                
                // Apply search filter to count visible ROMs
                std::string searchStr = m_searchBuffer;
                std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);
                
                std::vector<std::string> visibleRomPaths;
                for (const auto& systemPair : romsBySystem) {
                    const std::vector<RomInfo>& roms = systemPair.second;
                    for (const auto& rom : roms) {
                        if (searchStr.length() > 0) {
                            std::string romName = rom.displayName;
                            std::transform(romName.begin(), romName.end(), romName.begin(), ::tolower);
                            if (romName.find(searchStr) == std::string::npos) {
                                continue;
                            }
                        }
                        visibleRomPaths.push_back(rom.fullPath);
                    }
                }
                
                m_totalVisibleRoms = visibleRomPaths.size();
                if (m_totalVisibleRoms == 0) return;
                
                // Find current selection index
                int currentIndex = -1;
                if (!selectedRom.empty()) {
                    for (size_t i = 0; i < visibleRomPaths.size(); ++i) {
                        if (visibleRomPaths[i] == selectedRom) {
                            currentIndex = i;
                            break;
                        }
                    }
                }
                
                // Handle navigation
                bool navigated = false;
                if (navState.upPressed) {
                    if (currentIndex > 0) {
                        currentIndex--;
                        navigated = true;
                    }
                } else if (navState.downPressed) {
                    if (currentIndex < m_totalVisibleRoms - 1) {
                        currentIndex++;
                        navigated = true;
                    } else if (currentIndex == -1 && m_totalVisibleRoms > 0) {
                        currentIndex = 0;
                        navigated = true;
                    }
                }
                
                if (navigated) {
                    selectedRom = visibleRomPaths[currentIndex];
                    m_lastNavTime = currentTime;
                    m_selectedRomIndex = currentIndex;
                }
                
                // Handle confirm (launch game)
                if (navState.confirmPressed && !selectedRom.empty()) {
                    shouldLaunch = true;
                    m_lastNavTime = currentTime;
                }
            }
            break;
            
        case UIFocusMode::Buttons:
            // Build list of available buttons
            m_availableButtons.clear();
            if (!selectedRom.empty()) {
                m_availableButtons.push_back(ButtonType::LaunchGame);
            }
            m_availableButtons.push_back(ButtonType::RefreshList);
            m_availableButtons.push_back(ButtonType::Settings);
            m_availableButtons.push_back(ButtonType::Exit);
            
            // Clamp selection
            if (m_selectedButtonIndex >= (int)m_availableButtons.size()) {
                m_selectedButtonIndex = m_availableButtons.size() - 1;
            }
            if (m_selectedButtonIndex < 0) {
                m_selectedButtonIndex = 0;
            }
            
            // Navigate buttons
            if (navState.leftPressed && m_selectedButtonIndex > 0) {
                m_selectedButtonIndex--;
                m_lastNavTime = currentTime;
            } else if (navState.rightPressed && m_selectedButtonIndex < (int)m_availableButtons.size() - 1) {
                m_selectedButtonIndex++;
                m_lastNavTime = currentTime;
            }
            
            // Activate button
            if (navState.confirmPressed && m_selectedButtonIndex >= 0 && m_selectedButtonIndex < (int)m_availableButtons.size()) {
                ButtonType selectedButton = m_availableButtons[m_selectedButtonIndex];
                
                switch (selectedButton) {
                    case ButtonType::LaunchGame:
                        if (!selectedRom.empty()) {
                            shouldLaunch = true;
                        }
                        break;
                    case ButtonType::RefreshList:
                        romManager.scanDirectory("ROMS");
                        break;
                    case ButtonType::Settings:
                        showSettings = true;
                        break;
                    case ButtonType::Exit:
                        // Signal to quit the application
                        selectedRom = "";
                        shouldLaunch = false;
                        m_shouldQuit = true;
                        break;
                    default:
                        break;
                }
                
                m_lastNavTime = currentTime;
            }
            break;
            
        default:
            break;
    }
}

void GuiManager::handleSettingsNavigation(ControllerProfileManager& controllerManager, EmulatorConfig& config, bool& shouldApply) {
    if (!m_initialized) return;
    
    // Get the active controller (first one with input)
    int instanceId = controllerManager.getActiveController();
    if (instanceId < 0) return;
    
    auto navState = controllerManager.getUINavState(instanceId);
    
    // Get current time for input repeat
    float currentTime = SDL_GetTicks() / 1000.0f;
    bool canNavigate = (currentTime - m_lastNavTime) > m_navRepeatDelay;
    
    if (!canNavigate) return;
    
    // Clamp settings index
    if (m_settingsItemIndex < 0) m_settingsItemIndex = 0;
    if (m_settingsItemIndex >= m_settingsTotalItems) m_settingsItemIndex = m_settingsTotalItems - 1;
    
    // Navigate up/down
    if (navState.upPressed && m_settingsItemIndex > 0) {
        m_settingsItemIndex--;
        m_lastNavTime = currentTime;
    } else if (navState.downPressed && m_settingsItemIndex < m_settingsTotalItems - 1) {
        m_settingsItemIndex++;
        m_lastNavTime = currentTime;
    }
    
    // Adjust values left/right
    bool valueChanged = false;
    
    switch (m_settingsItemIndex) {
        case 0: // Window Scale
            if (navState.leftPressed && config.windowScale > 1) {
                config.windowScale--;
                valueChanged = true;
            } else if (navState.rightPressed && config.windowScale < 6) {
                config.windowScale++;
                valueChanged = true;
            }
            break;
            
        case 1: // Internal Resolution Scale
            if (navState.leftPressed && config.internalScale > 1) {
                config.internalScale--;
                valueChanged = true;
            } else if (navState.rightPressed && config.internalScale < 4) {
                config.internalScale++;
                valueChanged = true;
            }
            break;
            
        case 2: // VSync
            if (navState.confirmPressed) {
                config.vsync = !config.vsync;
                valueChanged = true;
            }
            break;
            
        case 3: // Linear Filtering
            if (navState.confirmPressed) {
                config.linearFilter = !config.linearFilter;
                valueChanged = true;
            }
            break;
            
        case 4: // Show FPS
            if (navState.confirmPressed) {
                config.showFPS = !config.showFPS;
                valueChanged = true;
            }
            break;
            
        case 5: // Volume
            if (navState.leftPressed && config.audioVolume > 0.0f) {
                config.audioVolume -= 0.05f;
                if (config.audioVolume < 0.0f) config.audioVolume = 0.0f;
                valueChanged = true;
            } else if (navState.rightPressed && config.audioVolume < 1.0f) {
                config.audioVolume += 0.05f;
                if (config.audioVolume > 1.0f) config.audioVolume = 1.0f;
                valueChanged = true;
            }
            break;
            
        case 6: // Apply & Close button
            if (navState.confirmPressed) {
                shouldApply = true;
                valueChanged = true;
            }
            break;
    }
    
    if (valueChanged) {
        m_lastNavTime = currentTime;
    }
    
    // Circle button = Apply & Close
    if (navState.cancelPressed) {
        shouldApply = true;
        m_lastNavTime = currentTime;
    }
}

