#include "controller_profile.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

ControllerProfileManager::ControllerProfileManager()
    : m_mappingInstanceId(-1)
    , m_isMappingMode(false)
    , m_profilesDir("./controller_profiles")
{
}

ControllerProfileManager::~ControllerProfileManager() {
    shutdown();
}

bool ControllerProfileManager::init() {
    // Create profiles directory if it doesn't exist
    if (!fs::exists(m_profilesDir)) {
        fs::create_directories(m_profilesDir);
    }
    
    // Initialize SDL game controller subsystem (if not already done)
    if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
            std::cerr << "Failed to initialize SDL game controller subsystem: " << SDL_GetError() << std::endl;
            return false;
        }
    }
    
    // Load custom game controller mappings
    int mappingsLoaded = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
    if (mappingsLoaded > 0) {
        std::cout << "Loaded " << mappingsLoaded << " game controller mapping(s) from gamecontrollerdb.txt" << std::endl;
    } else if (mappingsLoaded < 0) {
        std::cout << "Note: Could not load gamecontrollerdb.txt: " << SDL_GetError() << std::endl;
    }
    
    // Detect connected controllers
    detectControllers();
    
    // Load UI profile
    loadUIProfile();
    
    std::cout << "Controller profile manager initialized" << std::endl;
    return true;
}

void ControllerProfileManager::shutdown() {
    // Save UI profile
    saveUIProfile();
    
    // Close all controllers
    for (auto& pair : m_controllers) {
        // Controllers are managed by SDL, we just track them
    }
    m_controllers.clear();
}

void ControllerProfileManager::detectControllers() {
    m_controllers.clear();
    
    int numJoysticks = SDL_NumJoysticks();
    std::cout << "Detecting controllers: " << numJoysticks << " joystick(s) found" << std::endl;
    
    for (int i = 0; i < numJoysticks; ++i) {
        std::cout << "  Checking joystick " << i << "..." << std::endl;
        if (SDL_IsGameController(i)) {
            std::cout << "    Is a game controller" << std::endl;
            SDL_GameController* controller = SDL_GameControllerOpen(i);
            if (controller) {
                ControllerMapping mapping;
                mapping.name = SDL_GameControllerName(controller);
                mapping.guid = getControllerGUID(controller);
                mapping.deviceIndex = i;
                mapping.instanceId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
                
                m_controllers[mapping.instanceId] = mapping;
                
                std::cout << "    Controller " << i << ": " << mapping.name 
                         << " (Instance ID: " << mapping.instanceId << ", GUID: " << mapping.guid << ")" << std::endl;
                
                // Don't close - keep it open for the session
                // SDL_GameControllerClose(controller);
            } else {
                std::cout << "    Failed to open controller" << std::endl;
            }
        } else {
            std::cout << "    Not a game controller (joystick only)" << std::endl;
        }
    }
    
    std::cout << "detectControllers complete: " << m_controllers.size() << " controller(s) in m_controllers" << std::endl;
}

std::vector<ControllerMapping> ControllerProfileManager::getConnectedControllers() {
    std::vector<ControllerMapping> controllers;
    for (const auto& pair : m_controllers) {
        controllers.push_back(pair.second);
    }
    return controllers;
}

ControllerMapping* ControllerProfileManager::getController(int instanceId) {
    auto it = m_controllers.find(instanceId);
    if (it != m_controllers.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string ControllerProfileManager::getControllerGUID(SDL_GameController* controller) {
    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);
    char guidStr[33];
    SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
    return std::string(guidStr);
}

std::string ControllerProfileManager::getProfilePath(const std::string& profileName) {
    return m_profilesDir + "/" + profileName + ".json";
}

std::string ControllerProfileManager::getUIProfilePath() {
    return m_profilesDir + "/ui_profile.json";
}

bool ControllerProfileManager::loadProfile(const std::string& profileName) {
    std::string path = getProfilePath(profileName);
    ControllerProfile profile;
    
    if (loadProfileFromFile(path, profile)) {
        m_profiles[profileName] = profile;
        m_activeProfileName = profileName;
        std::cout << "Loaded controller profile: " << profileName << std::endl;
        return true;
    }
    
    return false;
}

bool ControllerProfileManager::saveProfile(const std::string& profileName) {
    auto it = m_profiles.find(profileName);
    if (it == m_profiles.end()) {
        std::cerr << "Profile not found: " << profileName << std::endl;
        return false;
    }
    
    std::string path = getProfilePath(profileName);
    return saveProfileToFile(path, it->second);
}

bool ControllerProfileManager::createProfile(const std::string& profileName, const std::string& coreName) {
    ControllerProfile profile;
    profile.profileName = profileName;
    profile.coreName = coreName;
    
    m_profiles[profileName] = profile;
    m_activeProfileName = profileName;
    
    std::cout << "Created controller profile: " << profileName << std::endl;
    return true;
}

void ControllerProfileManager::setActiveProfile(const std::string& profileName) {
    m_activeProfileName = profileName;
}

ControllerProfile* ControllerProfileManager::getActiveProfile() {
    if (m_activeProfileName.empty()) {
        return nullptr;
    }
    
    auto it = m_profiles.find(m_activeProfileName);
    if (it != m_profiles.end()) {
        return &it->second;
    }
    
    return nullptr;
}

std::vector<std::string> ControllerProfileManager::listProfiles() {
    std::vector<std::string> profiles;
    
    if (!fs::exists(m_profilesDir)) {
        return profiles;
    }
    
    for (const auto& entry : fs::directory_iterator(m_profilesDir)) {
        if (entry.path().extension() == ".json") {
            std::string filename = entry.path().stem().string();
            if (filename != "ui_profile") {
                profiles.push_back(filename);
            }
        }
    }
    
    return profiles;
}

bool ControllerProfileManager::loadUIProfile() {
    std::string path = getUIProfilePath();
    if (loadProfileFromFile(path, m_uiProfile)) {
        std::cout << "Loaded UI controller profile" << std::endl;
        return true;
    }
    
    // Create default UI profile if none exists
    m_uiProfile.profileName = "ui_profile";
    m_uiProfile.coreName = "UI";
    return false;
}

bool ControllerProfileManager::saveUIProfile() {
    std::string path = getUIProfilePath();
    return saveProfileToFile(path, m_uiProfile);
}

ControllerMapping* ControllerProfileManager::getUIMapping(int instanceId) {
    // First try direct instance ID lookup
    auto it = m_uiProfile.controllerMappings.find(instanceId);
    if (it != m_uiProfile.controllerMappings.end()) {
        return &it->second;
    }
    
    // If not found, try to match by GUID
    SDL_GameController* controller = SDL_GameControllerFromInstanceID(instanceId);
    if (controller) {
        SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
        if (joystick) {
            SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);
            char guidStr[64];
            SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));
            std::string currentGuid(guidStr);
            
            // Search all mappings for matching GUID
            for (auto& [id, mapping] : m_uiProfile.controllerMappings) {
                if (mapping.guid == currentGuid) {
                    // Cache this mapping with the current instance ID for faster lookup next time
                    m_uiProfile.controllerMappings[instanceId] = mapping;
                    return &m_uiProfile.controllerMappings[instanceId];
                }
            }
        }
    }
    
    return nullptr;
}

void ControllerProfileManager::setUIMapping(int instanceId, const ControllerMapping& mapping) {
    m_uiProfile.controllerMappings[instanceId] = mapping;
}

bool ControllerProfileManager::loadProfileFromFile(const std::string& filePath, ControllerProfile& profile) {
    if (!fs::exists(filePath)) {
        return false;
    }
    
    try {
        std::ifstream file(filePath);
        json j;
        file >> j;
        
        profile.profileName = j.value("profileName", "");
        profile.coreName = j.value("coreName", "");
        
        if (j.contains("controllers")) {
            for (auto& [key, value] : j["controllers"].items()) {
                int instanceId = std::stoi(key);
                ControllerMapping mapping;
                
                mapping.name = value.value("name", "");
                mapping.guid = value.value("guid", "");
                mapping.instanceId = instanceId;
                
                // Load button mappings
                if (value.contains("buttons")) {
                    for (auto& [btnKey, action] : value["buttons"].items()) {
                        mapping.buttonMap[std::stoi(btnKey)] = action;
                    }
                }
                
                // Load axis mappings
                if (value.contains("axes")) {
                    for (auto& [axisKey, action] : value["axes"].items()) {
                        mapping.axisMap[std::stoi(axisKey)] = action;
                    }
                }
                
                // Load UI mappings
                mapping.uiUp = value.value("uiUp", -1);
                mapping.uiDown = value.value("uiDown", -1);
                mapping.uiLeft = value.value("uiLeft", -1);
                mapping.uiRight = value.value("uiRight", -1);
                mapping.uiConfirm = value.value("uiConfirm", -1);
                mapping.uiCancel = value.value("uiCancel", -1);
                mapping.uiMenu = value.value("uiMenu", -1);
                mapping.uiAxisX = value.value("uiAxisX", -1);
                mapping.uiAxisY = value.value("uiAxisY", -1);
                mapping.uiDeadzone = value.value("uiDeadzone", 0.3f);
                mapping.uiUseHat = value.value("uiUseHat", false);
                mapping.uiHat = value.value("uiHat", 0);
                
                profile.controllerMappings[instanceId] = mapping;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading profile from " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

bool ControllerProfileManager::saveProfileToFile(const std::string& filePath, const ControllerProfile& profile) {
    try {
        json j;
        j["profileName"] = profile.profileName;
        j["coreName"] = profile.coreName;
        
        json controllers = json::object();
        for (const auto& [instanceId, mapping] : profile.controllerMappings) {
            json controller;
            controller["name"] = mapping.name;
            controller["guid"] = mapping.guid;
            
            // Save button mappings
            json buttons = json::object();
            for (const auto& [btn, action] : mapping.buttonMap) {
                buttons[std::to_string(btn)] = action;
            }
            if (!buttons.empty()) {
                controller["buttons"] = buttons;
            }
            
            // Save axis mappings
            json axes = json::object();
            for (const auto& [axis, action] : mapping.axisMap) {
                axes[std::to_string(axis)] = action;
            }
            if (!axes.empty()) {
                controller["axes"] = axes;
            }
            
            // Save UI mappings
            controller["uiUp"] = mapping.uiUp;
            controller["uiDown"] = mapping.uiDown;
            controller["uiLeft"] = mapping.uiLeft;
            controller["uiRight"] = mapping.uiRight;
            controller["uiConfirm"] = mapping.uiConfirm;
            controller["uiCancel"] = mapping.uiCancel;
            controller["uiMenu"] = mapping.uiMenu;
            controller["uiAxisX"] = mapping.uiAxisX;
            controller["uiAxisY"] = mapping.uiAxisY;
            controller["uiDeadzone"] = mapping.uiDeadzone;
            controller["uiUseHat"] = mapping.uiUseHat;
            controller["uiHat"] = mapping.uiHat;
            
            controllers[std::to_string(instanceId)] = controller;
        }
        j["controllers"] = controllers;
        
        std::ofstream file(filePath);
        file << j.dump(2);
        
        std::cout << "Saved controller profile to " << filePath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving profile to " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

void ControllerProfileManager::startMapping(int instanceId) {
    m_mappingInstanceId = instanceId;
    m_isMappingMode = true;
    std::cout << "Started mapping mode for controller instance " << instanceId << std::endl;
}

bool ControllerProfileManager::mapButton(int button, const std::string& action) {
    if (!m_isMappingMode || m_mappingInstanceId < 0) {
        return false;
    }
    
    auto* mapping = getController(m_mappingInstanceId);
    if (!mapping) {
        return false;
    }
    
    mapping->buttonMap[button] = action;
    std::cout << "Mapped button " << button << " to action: " << action << std::endl;
    return true;
}

bool ControllerProfileManager::mapAxis(int axis, const std::string& action) {
    if (!m_isMappingMode || m_mappingInstanceId < 0) {
        return false;
    }
    
    auto* mapping = getController(m_mappingInstanceId);
    if (!mapping) {
        return false;
    }
    
    mapping->axisMap[axis] = action;
    std::cout << "Mapped axis " << axis << " to action: " << action << std::endl;
    return true;
}

void ControllerProfileManager::endMapping() {
    m_isMappingMode = false;
    m_mappingInstanceId = -1;
    std::cout << "Ended mapping mode" << std::endl;
}

bool ControllerProfileManager::handleControllerButton(int instanceId, int button, bool pressed) {
    // If in mapping mode, just record the button
    if (m_isMappingMode && instanceId == m_mappingInstanceId) {
        // Handled by the UI mapping screen
        return true;
    }
    
    // Otherwise, check if it's a UI button
    auto* uiMapping = getUIMapping(instanceId);
    if (uiMapping) {
        // UI navigation handled elsewhere
        return false; // Let caller handle it
    }
    
    return false;
}

bool ControllerProfileManager::handleControllerAxis(int instanceId, int axis, float value) {
    // Similar to button handling
    return false;
}

int ControllerProfileManager::getActiveController() {
    std::cout << "getActiveController: m_controllers.size() = " << m_controllers.size() << std::endl;
    
    // Check all connected controllers and return the first one with any input
    for (const auto& [instanceId, mapping] : m_controllers) {
        UINavState state = getUINavState(instanceId);
        
        // Check if any button or direction is pressed
        if (state.upPressed || state.downPressed || state.leftPressed || state.rightPressed ||
            state.confirmPressed || state.cancelPressed || state.menuPressed ||
            state.shoulderLeftPressed || state.shoulderRightPressed) {
            std::cout << "Active controller found: Instance ID " << instanceId 
                     << " (" << mapping.name << ")" << std::endl;
            return instanceId;
        }
    }
    
    // If no controller has input, return the first one (or -1 if none)
    if (!m_controllers.empty()) {
        int firstId = m_controllers.begin()->first;
        std::cout << "No input detected, using first controller: Instance ID " << firstId 
                 << " (" << m_controllers.begin()->second.name << ")" << std::endl;
        return firstId;
    }
    
    std::cout << "No controllers connected!" << std::endl;
    return -1;
}

ControllerProfileManager::UINavState ControllerProfileManager::getUINavState(int instanceId) {
    UINavState state;
    
    auto* uiMapping = getUIMapping(instanceId);
    if (!uiMapping) {
        std::cout << "No UI mapping found for instance ID " << instanceId << std::endl;
        return state;
    }
    
    std::cout << "UI mapping found for instance ID " << instanceId 
             << " (GUID: " << uiMapping->guid << ", Name: " << uiMapping->name 
             << ", UseHAT: " << (uiMapping->uiUseHat ? "true" : "false") << ")" << std::endl;
    
    SDL_GameController* controller = SDL_GameControllerFromInstanceID(instanceId);
    if (!controller) {
        std::cout << "Failed to get SDL_GameController for instance ID " << instanceId << std::endl;
        return state;
    }
    
    // Check button-based navigation
    if (uiMapping->uiUp >= 0) {
        state.upPressed = SDL_GameControllerGetButton(controller, static_cast<SDL_GameControllerButton>(uiMapping->uiUp));
    }
    if (uiMapping->uiDown >= 0) {
        state.downPressed = SDL_GameControllerGetButton(controller, static_cast<SDL_GameControllerButton>(uiMapping->uiDown));
    }
    if (uiMapping->uiLeft >= 0) {
        state.leftPressed = SDL_GameControllerGetButton(controller, static_cast<SDL_GameControllerButton>(uiMapping->uiLeft));
    }
    if (uiMapping->uiRight >= 0) {
        state.rightPressed = SDL_GameControllerGetButton(controller, static_cast<SDL_GameControllerButton>(uiMapping->uiRight));
    }
    if (uiMapping->uiConfirm >= 0) {
        state.confirmPressed = SDL_GameControllerGetButton(controller, static_cast<SDL_GameControllerButton>(uiMapping->uiConfirm));
    }
    if (uiMapping->uiCancel >= 0) {
        state.cancelPressed = SDL_GameControllerGetButton(controller, static_cast<SDL_GameControllerButton>(uiMapping->uiCancel));
    }
    if (uiMapping->uiMenu >= 0) {
        state.menuPressed = SDL_GameControllerGetButton(controller, static_cast<SDL_GameControllerButton>(uiMapping->uiMenu));
    }
    
    // Check shoulder buttons for tab switching
    state.shoulderLeftPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    state.shoulderRightPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    
    // Check axis-based navigation (analog stick)
    if (uiMapping->uiAxisX >= 0) {
        float axisValue = SDL_GameControllerGetAxis(controller, static_cast<SDL_GameControllerAxis>(uiMapping->uiAxisX)) / 32767.0f;
        if (axisValue < -uiMapping->uiDeadzone) {
            state.leftPressed = true;
        } else if (axisValue > uiMapping->uiDeadzone) {
            state.rightPressed = true;
        }
    }
    if (uiMapping->uiAxisY >= 0) {
        float axisValue = SDL_GameControllerGetAxis(controller, static_cast<SDL_GameControllerAxis>(uiMapping->uiAxisY)) / 32767.0f;
        if (axisValue < -uiMapping->uiDeadzone) {
            state.upPressed = true;
        } else if (axisValue > uiMapping->uiDeadzone) {
            state.downPressed = true;
        }
    }
    
    // Check HAT-based navigation (D-pad via HAT)
    if (uiMapping->uiUseHat) {
        SDL_Joystick* joy = SDL_GameControllerGetJoystick(controller);
        if (joy && uiMapping->uiHat < SDL_JoystickNumHats(joy)) {
            Uint8 hatValue = SDL_JoystickGetHat(joy, uiMapping->uiHat);
            if (hatValue & SDL_HAT_UP) {
                state.upPressed = true;
            }
            if (hatValue & SDL_HAT_DOWN) {
                state.downPressed = true;
            }
            if (hatValue & SDL_HAT_LEFT) {
                state.leftPressed = true;
            }
            if (hatValue & SDL_HAT_RIGHT) {
                state.rightPressed = true;
            }
        }
    }
    
    return state;
}

void ControllerProfileManager::createDefaultUIProfile() {
    // Create a default UI navigation profile with standard Xbox/PlayStation-style mappings
    m_uiProfile.profileName = "UI Navigation";
    m_uiProfile.coreName = "ui";
    
    // Get the first connected controller (if any)
    auto controllers = getConnectedControllers();
    if (controllers.empty()) {
        std::cout << "No controllers detected for default UI profile" << std::endl;
        return;
    }
    
    // Create default mapping for the first controller
    ControllerMapping defaultMapping = controllers[0]; // Copy basic info
    
    // D-Pad for UI navigation
    defaultMapping.uiUp = SDL_CONTROLLER_BUTTON_DPAD_UP;
    defaultMapping.uiDown = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
    defaultMapping.uiLeft = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
    defaultMapping.uiRight = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
    
    // Left analog stick for UI navigation
    defaultMapping.uiAxisX = SDL_CONTROLLER_AXIS_LEFTX;
    defaultMapping.uiAxisY = SDL_CONTROLLER_AXIS_LEFTY;
    defaultMapping.uiDeadzone = 0.3f;
    
    // A/Cross button for confirm
    defaultMapping.uiConfirm = SDL_CONTROLLER_BUTTON_A;
    
    // B/Circle button for cancel/back
    defaultMapping.uiCancel = SDL_CONTROLLER_BUTTON_B;
    
    // Start button for menu
    defaultMapping.uiMenu = SDL_CONTROLLER_BUTTON_START;
    
    // Standard libretro button mappings
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_A] = "b";          // SNES A -> B button
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_B] = "a";          // SNES B -> A button
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_X] = "y";          // SNES X -> Y button
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_Y] = "x";          // SNES Y -> X button
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = "l";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = "r";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_LEFTSTICK] = "l3";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = "r3";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_BACK] = "select";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_START] = "start";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_DPAD_UP] = "up";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = "down";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = "left";
    defaultMapping.buttonMap[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = "right";
    
    defaultMapping.axisMap[SDL_CONTROLLER_AXIS_LEFTX] = "leftx";
    defaultMapping.axisMap[SDL_CONTROLLER_AXIS_LEFTY] = "lefty";
    defaultMapping.axisMap[SDL_CONTROLLER_AXIS_RIGHTX] = "rightx";
    defaultMapping.axisMap[SDL_CONTROLLER_AXIS_RIGHTY] = "righty";
    defaultMapping.axisMap[SDL_CONTROLLER_AXIS_TRIGGERLEFT] = "l2";
    defaultMapping.axisMap[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] = "r2";
    
    m_uiProfile.controllerMappings[defaultMapping.instanceId] = defaultMapping;
    
    std::cout << "Created default UI navigation profile for controller: " << defaultMapping.name << std::endl;
}

bool ControllerProfileManager::isExitComboPressed(int instanceId) {
    SDL_GameController* controller = SDL_GameControllerFromInstanceID(instanceId);
    if (!controller) {
        return false;
    }
    
    // Check if both Start and Select (Back) buttons are pressed
    bool startPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
    bool selectPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
    
    return startPressed && selectPressed;
}

