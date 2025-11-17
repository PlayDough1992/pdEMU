#ifndef CONTROLLER_PROFILE_H
#define CONTROLLER_PROFILE_H

#include <SDL2/SDL.h>
#include <string>
#include <map>
#include <vector>

// Controller button/axis mapping for a specific controller
struct ControllerMapping {
    // Physical controller info
    std::string name;
    std::string guid;
    int instanceId;
    int deviceIndex;
    
    // Button mappings (SDL button -> action)
    std::map<int, std::string> buttonMap;
    
    // Axis mappings (SDL axis -> action)
    std::map<int, std::string> axisMap;
    
    // UI navigation mappings
    int uiUp = -1;
    int uiDown = -1;
    int uiLeft = -1;
    int uiRight = -1;
    int uiConfirm = -1;
    int uiCancel = -1;
    int uiMenu = -1;
    
    // Axis for UI (if using analog stick)
    int uiAxisX = -1;
    int uiAxisY = -1;
    float uiDeadzone = 0.3f;
    
    // HAT for UI (if using HAT for D-pad)
    bool uiUseHat = false;
    int uiHat = 0;
};

// Profile for a specific game/core
struct ControllerProfile {
    std::string profileName;
    std::string coreName;
    std::map<int, ControllerMapping> controllerMappings; // controllerId -> mapping
};

class ControllerProfileManager {
public:
    ControllerProfileManager();
    ~ControllerProfileManager();
    
    // Initialize controller subsystem
    bool init();
    void shutdown();
    
    // Controller detection
    void detectControllers();
    std::vector<ControllerMapping> getConnectedControllers();
    ControllerMapping* getController(int instanceId);
    
    // Profile management
    bool loadProfile(const std::string& profileName);
    bool saveProfile(const std::string& profileName);
    bool createProfile(const std::string& profileName, const std::string& coreName = "");
    void setActiveProfile(const std::string& profileName);
    ControllerProfile* getActiveProfile();
    std::vector<std::string> listProfiles();
    
    // UI profile (global, for menu navigation)
    bool loadUIProfile();
    bool saveUIProfile();
    ControllerMapping* getUIMapping(int instanceId);
    void setUIMapping(int instanceId, const ControllerMapping& mapping);
    
    // Mapping configuration
    void startMapping(int instanceId);
    bool mapButton(int button, const std::string& action);
    bool mapAxis(int axis, const std::string& action);
    void endMapping();
    
    // Input handling
    bool handleControllerButton(int instanceId, int button, bool pressed);
    bool handleControllerAxis(int instanceId, int axis, float value);
    
    // UI navigation state
    struct UINavState {
        bool upPressed = false;
        bool downPressed = false;
        bool leftPressed = false;
        bool rightPressed = false;
        bool confirmPressed = false;
        bool cancelPressed = false;
        bool menuPressed = false;
        bool shoulderLeftPressed = false;
        bool shoulderRightPressed = false;
    };
    
    UINavState getUINavState(int instanceId);
    
    // Get the first controller that has any input (for determining active player)
    int getActiveController();
    
    // Check for exit combo (Start + Select pressed simultaneously)
    bool isExitComboPressed(int instanceId);
    
    // Create default profiles
    void createDefaultUIProfile();
    
private:
    std::map<int, ControllerMapping> m_controllers; // instanceId -> controller
    std::map<std::string, ControllerProfile> m_profiles; // profileName -> profile
    ControllerProfile m_uiProfile; // Global UI navigation profile
    std::string m_activeProfileName;
    std::string m_profilesDir;
    
    int m_mappingInstanceId;
    bool m_isMappingMode;
    
    // Helper functions
    std::string getControllerGUID(SDL_GameController* controller);
    std::string getProfilePath(const std::string& profileName);
    std::string getUIProfilePath();
    bool loadProfileFromFile(const std::string& filePath, ControllerProfile& profile);
    bool saveProfileToFile(const std::string& filePath, const ControllerProfile& profile);
};

#endif // CONTROLLER_PROFILE_H
