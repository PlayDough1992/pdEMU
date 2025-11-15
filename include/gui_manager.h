#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include "config_manager.h"
#include "rom_manager.h"
#include "controller_profile.h"
#include <SDL2/SDL.h>
#include <string>


class GuiManager {
public:
    GuiManager();
    ~GuiManager();

    bool init(SDL_Window* window, SDL_Renderer* renderer);
    void shutdown();

    void beginFrame();
    void endFrame(SDL_Renderer* renderer);

    void renderMainMenu(bool& showMenu, bool& showSettings);
    void renderRomBrowser(RomManager& romManager, std::string& selectedRom, bool& shouldLaunch, bool& showSettings);
    void renderSettings(EmulatorConfig& config, bool& shouldApply);
    void renderFPSCounter(float fps);

    // Controller navigation
    void handleControllerNavigation(ControllerProfileManager& controllerManager, RomManager& romManager, std::string& selectedRom, bool& shouldLaunch, bool& showSettings);
    void handleSettingsNavigation(ControllerProfileManager& controllerManager, EmulatorConfig& config, bool& shouldApply);
    
    // Check if user wants to quit
    bool shouldQuit() const { return m_shouldQuit; }
    void setShouldQuit(bool quit) { m_shouldQuit = quit; }

    bool wantsCaptureMouse() const;
    bool wantsCaptureKeyboard() const;

    void processEvent(SDL_Event* event);

    bool isInitialized() const { return m_initialized; }

     // Debug mode toggle for skipping/ignoring OpenGL errors
     bool getDebugMode() const { return m_debugMode; }
     void setDebugMode(bool debug) { m_debugMode = debug; }

private:
    bool m_initialized;
    void* m_imguiContext;

    // UI state
    int m_selectedRomIndex;
    bool m_showAbout;
    char m_searchBuffer[256];
    
    // Controller navigation state
    float m_lastNavTime;
    float m_navRepeatDelay;
    int m_totalVisibleRoms;
    
    // UI focus management
    enum class UIFocusMode {
        RomList,
        SearchBox,
        Buttons,
        OnScreenKeyboard
    };
    
    UIFocusMode m_focusMode;
    int m_buttonFocusIndex;
    bool m_showOnScreenKeyboard;
    std::string m_keyboardInput;
    
    // Button navigation
    enum class ButtonType {
        LaunchGame,
        RefreshList,
        Settings,
        Exit,
        ChangeSystem,
        DebugCheckbox
    };
    
    std::vector<ButtonType> m_availableButtons;
    int m_selectedButtonIndex;
    
    // Quit flag
    bool m_shouldQuit;
    
    // Settings navigation
    int m_settingsItemIndex;
    int m_settingsTotalItems;
    
    // On-screen keyboard navigation
    int m_keyboardRow;
    int m_keyboardCol;
    
    // On-screen keyboard
    void renderOnScreenKeyboard(bool& shouldClose, bool& shouldApply);

    // Dolphin backend selection (0 = OpenGL, 1 = Vulkan)
    int m_dolphinBackend = 0;
    std::string m_lastDolphinRom;
     bool m_debugMode = false;
public:
    int getDolphinBackend() const { return m_dolphinBackend; }
    void setDolphinBackend(int backend) { m_dolphinBackend = backend; }
    void setLastDolphinRom(const std::string& rom) { m_lastDolphinRom = rom; }
    std::string getLastDolphinRom() const { return m_lastDolphinRom; }
};

#endif // GUI_MANAGER_H
