#include "libretro_core.h"
#include "video_renderer.h"
#include "video_renderer_gl.h"
#include "audio_renderer.h"
#include "input_handler.h"
#include "config_manager.h"
#include "rom_manager.h"
#include "gui_manager.h"
#include "system_database.h"
#include "core_manager.h"
#include "splash_screen.h"
#include "controller_profile.h"
#include <SDL2/SDL_opengl.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#endif

// Define missing GL constants if not available
#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

// Global pointers for callbacks
static VideoRenderer* g_videoRenderer = nullptr;
static VideoRendererGL* g_videoRendererGL = nullptr;
static AudioRenderer* g_audioRenderer = nullptr;
static InputHandler* g_inputHandler = nullptr;
static bool g_useOpenGL = false;
static bool g_shouldPresent = false;  // Flag set by video_refresh when core renders a valid frame
static bool g_shouldClear = false;    // Flag set when we should clear before next render
static bool g_isDK64 = false;         // Flag for DK64-specific frame dupe handling

// These need external linkage for libretro_core.cpp
SDL_Window* g_gameWindow = nullptr;
unsigned g_gameWidth = 640;
unsigned g_gameHeight = 480;

// Libretro callbacks
void video_refresh_callback(const void* data, unsigned width, unsigned height, size_t pitch) {
    static int frameCounter = 0;
    static int nullCounter = 0;
    
    if (g_useOpenGL && g_videoRendererGL) {
        // For hardware rendering, check for RETRO_HW_FRAME_BUFFER_VALID (-1) or NULL
        // RETRO_HW_FRAME_BUFFER_VALID means the core rendered to the FBO
        if (data == RETRO_HW_FRAME_BUFFER_VALID) {
            // Core rendered a new frame - we should present it
            g_shouldPresent = true;
            frameCounter++;
        } else if (data == nullptr) {
            // Frame dupe/null
            nullCounter++;
            if (nullCounter % 60 == 0) {
                std::cout << "NULL frames: " << nullCounter << " (DK64 mode: " << (g_isDK64 ? "yes" : "no") << ")" << std::endl;
            }
            
            if (g_isDK64) {
                g_shouldPresent = false;  // DK64: skip to avoid black flicker
            } else {
                g_shouldPresent = true;   // Other games: present anyway (shouldn't normally get NULLs)
            }
        } else {
            // Software fallback - shouldn't happen for hardware cores but handle it
            std::cerr << "WARNING: Hardware core provided pixel data instead of rendering to FBO!" << std::endl;
            g_videoRendererGL->updateFrame(data, width, height, pitch);
            g_shouldPresent = true;
        }
    } else if (!g_useOpenGL && g_videoRenderer && data) {
        g_videoRenderer->render(data, width, height, pitch);
    }
}

void audio_sample_callback(int16_t left, int16_t right) {
    if (g_audioRenderer) {
        g_audioRenderer->pushSample(left, right);
    }
}

size_t audio_sample_batch_callback(const int16_t* data, size_t frames) {
    if (g_audioRenderer) {
        g_audioRenderer->pushSamples(data, frames);
    }
    return frames;
}

void input_poll_callback() {
    if (g_inputHandler) {
        g_inputHandler->poll();
    }
}

int16_t input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id) {
    if (g_inputHandler) {
        return g_inputHandler->getInputState(port, device, index, id);
    }
    return 0;
}

// Get the directory where the executable is located
std::string getExecutableDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    // Find the last backslash
    size_t pos = exePath.find_last_of("\\/");
    if (pos != std::string::npos) {
        return exePath.substr(0, pos);
    }
    return ".";
#else
    // Linux implementation (if needed in future)
    return ".";
#endif
}

int main(int argc, char* argv[]) {
    // Get the executable directory for portable paths
    std::string exeDir = getExecutableDirectory();
    std::cout << "Executable directory: " << exeDir << std::endl;
    
    // Load config
    ConfigManager configManager;
    configManager.load();
    
    // Make paths relative to executable if they are not absolute
    auto& config = configManager.getConfig();
    
    // Store original paths before converting to absolute
    std::string origCoresPath = config.coresPath;
    std::string origRomsPath = config.romsPath;
    std::string origBiosPath = config.biosPath;
    std::string origSavesPath = config.savesPath;
    
    // Helper lambda to make path absolute if it's relative
    auto makeAbsolutePath = [&exeDir](std::string& path) {
        // Check if path is already absolute (starts with drive letter on Windows)
        if (path.length() >= 2 && path[1] == ':') {
            return; // Already absolute
        }
        // Check if path starts with ./ or ../
        if (path[0] != '/' && path[0] != '\\') {
            // It's relative, prepend executable directory
            path = exeDir + "\\" + path;
        }
    };
    
    makeAbsolutePath(config.coresPath);
    makeAbsolutePath(config.romsPath);
    makeAbsolutePath(config.biosPath);
    makeAbsolutePath(config.savesPath);
    
    // Store original relative paths for saving later
    configManager.setOriginalPaths(origCoresPath, origRomsPath, origBiosPath, origSavesPath);
    
    std::cout << "Using paths:" << std::endl;
    std::cout << "  Cores: " << config.coresPath << std::endl;
    std::cout << "  ROMs:  " << config.romsPath << std::endl;
    std::cout << "  BIOS:  " << config.biosPath << std::endl;
    std::cout << "  Saves: " << config.savesPath << std::endl;
    
    // Initialize system database
    SystemDatabase systemDb;
    systemDb.initialize();
    std::cout << "Initialized system database with " << systemDb.getAllSystemNames().size() << " systems" << std::endl;
    
    // Initialize core manager
    CoreManager coreManager;
    coreManager.scanCoresDirectory(configManager.getConfig().coresPath);
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize controller profile manager
    ControllerProfileManager controllerManager;
    if (!controllerManager.init()) {
        std::cerr << "Failed to initialize controller profile manager" << std::endl;
    }
    
    // Detect and log available controllers
    auto controllers = controllerManager.getConnectedControllers();
    std::cout << "Detected " << controllers.size() << " controller(s)" << std::endl;
    for (const auto& ctrl : controllers) {
        std::cout << "  - " << ctrl.name << " (Instance: " << ctrl.instanceId << ")" << std::endl;
    }
    
    // Load UI navigation profile
    if (!controllerManager.loadProfile("ui_profile")) {
        std::cout << "No UI navigation profile found, creating default..." << std::endl;
        controllerManager.createDefaultUIProfile();
        controllerManager.saveProfile("ui_profile");
    }

    // Get desktop resolution for fullscreen ROM browser
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        std::cerr << "Failed to get display mode: " << SDL_GetError() << std::endl;
        displayMode.w = 1920;
        displayMode.h = 1080;
    }

    // Create fullscreen borderless window for ROM browser
    SDL_Window* window = SDL_CreateWindow(
        "pdEMU - Universal Emulator",
        0, 0,
        displayMode.w, displayMode.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP
    );

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize GUI
    GuiManager guiManager;
    if (!guiManager.init(window, renderer)) {
        std::cerr << "Failed to initialize GUI" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize ROM manager
    RomManager romManager;
    romManager.setSystemDatabase(&systemDb);
    romManager.scanDirectory(configManager.getConfig().romsPath);

    // Main application loop - keeps running until user quits
    bool quit = false;
    
    std::cout << "ROM Browser started. Select a ROM to play!" << std::endl;

    while (!quit) {
        // ROM browser loop
        bool shouldLaunchGame = false;
        std::string selectedRomPath;
        bool showSettings = false;
        bool settingsShouldApply = false;

        while (!quit && !shouldLaunchGame) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (!g_useOpenGL && guiManager.isInitialized()) {
                guiManager.processEvent(&event);
            }
            
            // Handle controller events
            if (event.type == SDL_CONTROLLERDEVICEADDED) {
                controllerManager.detectControllers();
                std::cout << "Controller connected" << std::endl;
            } else if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
                controllerManager.detectControllers();
                std::cout << "Controller disconnected" << std::endl;
            }
            
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (event.key.keysym.sym == SDLK_F10) {
                    showSettings = !showSettings;
                } else if (event.key.keysym.sym == SDLK_F9) {
                    // Open controller configuration
                    std::cout << "Controller configuration not yet implemented" << std::endl;
                }
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
        SDL_RenderClear(renderer);

        guiManager.beginFrame();
        
        // Check if user clicked Exit button
        if (guiManager.shouldQuit()) {
            quit = true;
        }
        
        if (showSettings) {
            // Handle controller navigation in settings
            guiManager.handleSettingsNavigation(controllerManager, configManager.getConfig(), settingsShouldApply);
            
            guiManager.renderSettings(configManager.getConfig(), settingsShouldApply);
            if (settingsShouldApply) {
                configManager.save();
                settingsShouldApply = false;
                showSettings = false;
            }
        } else {
            // Handle controller navigation
            guiManager.handleControllerNavigation(controllerManager, romManager, selectedRomPath, shouldLaunchGame, showSettings);
            
            guiManager.renderRomBrowser(romManager, selectedRomPath, shouldLaunchGame, showSettings);
        }

        guiManager.endFrame(renderer);
        SDL_RenderPresent(renderer);
        
        SDL_Delay(16); // ~60 FPS
    }

    if (quit || selectedRomPath.empty()) {
        // User wants to quit, exit the main loop
        break;
    }

    // User selected a ROM, prepare to launch emulator
    // Cleanup ROM browser GUI temporarily
    guiManager.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Now start the actual emulator
    std::cout << "\nLaunching game: " << selectedRomPath << std::endl;
    
    // Determine which core to use based on ROM filename
    // Extract just the filename from the full path
    size_t slashPos = selectedRomPath.find_last_of('/');
    std::string filename = (slashPos != std::string::npos) ? selectedRomPath.substr(slashPos + 1) : selectedRomPath;
    
    // First check for system marker in filename (e.g., "_GCM", "_PS2")
    const SystemInfo* system = systemDb.getSystemByMarker(filename);
    
    // If no marker found, fall back to extension detection
    if (!system) {
        size_t dotPos = selectedRomPath.find_last_of('.');
        if (dotPos != std::string::npos) {
            std::string ext = selectedRomPath.substr(dotPos);
            system = systemDb.getSystemByExtension(ext);
        }
    }
    
    std::string corePath;
    
    if (system) {
        std::cout << "Detected system: " << system->displayName << std::endl;
        
        // Some systems are known to require OpenGL (GameCube/Wii), but others
        // might request it dynamically (N64, PS2, etc.). We'll create a temporary
        // OpenGL context and let the core request hardware rendering if needed.
        g_useOpenGL = (system->name == "gamecube" || system->name == "wii" || 
                      system->name == "n64" || system->name == "ps2" ||
                      system->name == "dreamcast");
        
        if (g_useOpenGL) {
            std::cout << "System may require OpenGL rendering" << std::endl;
        }
        
        corePath = coreManager.getBestCoreForSystem(system->name);
        
        if (corePath.empty()) {
            std::cerr << "No core found for " << system->displayName << std::endl;
            std::cerr << "Please download one of these cores:" << std::endl;
            for (const auto& coreFile : system->coreFiles) {
                std::cerr << "  - " << coreFile << std::endl;
            }
            // Return to ROM browser instead of exiting
            continue;
        }
        
        std::cout << "Using core: " << corePath << std::endl;
    } else {
        std::cerr << "Unknown file type: " << filename << std::endl;
        // Return to ROM browser instead of exiting
        continue;
    }

    // For hardware rendering systems, we need OpenGL context
    // active BEFORE calling retro_load_game
    // (This is different from software rendering cores)
    SDL_Window* tempGLWindow = nullptr;
    SDL_GLContext tempGLContext = nullptr;
    
    if (g_useOpenGL) {
        std::cout << "Creating temporary OpenGL context for hardware-rendered cores..." << std::endl;
        
        // Set OpenGL attributes - start with COMPATIBILITY for loading
        // We'll check what the core actually needs and may recreate the context
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        
        tempGLWindow = SDL_CreateWindow(
            "Loading...",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 480,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
        );
        
        if (!tempGLWindow) {
            std::cerr << "Failed to create temporary GL window: " << SDL_GetError() << std::endl;
            continue;
        }
        
        tempGLContext = SDL_GL_CreateContext(tempGLWindow);
        if (!tempGLContext) {
            std::cerr << "Failed to create temporary GL context: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(tempGLWindow);
            continue;
        }
        
        SDL_GL_MakeCurrent(tempGLWindow, tempGLContext);
        std::cout << "Temporary OpenGL context created successfully" << std::endl;
    }

    // Get desktop resolution for fullscreen splash
    SDL_DisplayMode splashDisplayMode;
    if (SDL_GetCurrentDisplayMode(0, &splashDisplayMode) != 0) {
        std::cerr << "Failed to get display mode: " << SDL_GetError() << std::endl;
        splashDisplayMode.w = 1920;
        splashDisplayMode.h = 1080;
    }

    // Create fullscreen borderless splash screen window
    SDL_Window* splashWindow = SDL_CreateWindow(
        "pdEMU",
        0, 0,
        splashDisplayMode.w, splashDisplayMode.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP
    );
    
    if (!splashWindow) {
        std::cerr << "Failed to create splash window: " << SDL_GetError() << std::endl;
        if (tempGLContext) SDL_GL_DeleteContext(tempGLContext);
        if (tempGLWindow) SDL_DestroyWindow(tempGLWindow);
        // Return to ROM browser instead of exiting
        continue;
    }
    
    SDL_Renderer* splashRenderer = SDL_CreateRenderer(splashWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!splashRenderer) {
        std::cerr << "Failed to create splash renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(splashWindow);
        // Return to ROM browser instead of exiting
        continue;
    }
    
    // Show splash screen
    SplashScreen splash;
    splash.init(splashWindow, splashRenderer);
    splash.show(2000, "Loading...");
    
    // Cleanup splash
    splash.shutdown();
    SDL_DestroyRenderer(splashRenderer);
    SDL_DestroyWindow(splashWindow);

    // Create emulator instances
    LibretroCore core;
    VideoRenderer videoRenderer;
    VideoRendererGL videoRendererGL;
    AudioRenderer audioRenderer;
    InputHandler inputHandler;

    // Set global pointers for callbacks
    g_videoRenderer = &videoRenderer;
    g_videoRendererGL = &videoRendererGL;
    g_audioRenderer = &audioRenderer;
    g_inputHandler = &inputHandler;


    // Set Dolphin backend if launching Dolphin core for GameCube/Wii
    std::string coreFileName = corePath.substr(corePath.find_last_of("/") + 1);
    if ((system && (system->name == "gamecube" || system->name == "wii")) &&
        (coreFileName == "dolphin_libretro.so")) {
        // Get backend selection from GUI
        int backendIdx = guiManager.getDolphinBackend();
        std::string backendStr = (backendIdx == 1) ? "Vulkan" : "OpenGL";
        extern std::map<std::string, std::string> g_coreVariables;
        g_coreVariables["dolphin_video_backend"] = backendStr;
        std::cout << "Set dolphin_video_backend to: " << backendStr << std::endl;
    }

    // Load the core (corePath was determined above)
    std::cout << "Loading core: " << corePath << std::endl;
    if (!core.loadCore(corePath)) {
        std::cerr << "Failed to load core" << std::endl;
        SDL_Quit();
        return 1;
    }
    std::cout << "Core loaded successfully" << std::endl;

    // Set up control scheme based on core name
    std::string coreName = corePath;
    size_t lastSlash = coreName.find_last_of('/');
    if (lastSlash != std::string::npos) {
        coreName = coreName.substr(lastSlash + 1);
    }
    inputHandler.setControlScheme(coreName);
    
    // Set the active controller for in-game input (the one that was used in UI)
    int activeControllerInstance = controllerManager.getActiveController();
    if (activeControllerInstance >= 0) {
        inputHandler.setActiveController(activeControllerInstance);
        std::cout << "Using controller instance " << activeControllerInstance << " for in-game input" << std::endl;
    } else {
        std::cout << "No active controller found, using keyboard or first controller" << std::endl;
    }

    // Set callbacks
    std::cout << "Setting up callbacks..." << std::endl;
    core.setVideoRefreshCallback(video_refresh_callback);
    core.setAudioSampleCallback(audio_sample_callback);
    core.setAudioSampleBatchCallback(audio_sample_batch_callback);
    core.setInputPollCallback(input_poll_callback);
    core.setInputStateCallback(input_state_callback);
    std::cout << "Callbacks set" << std::endl;

    // DON'T create OpenGL context yet - let the game load first
    // The core will request hardware rendering through environment callback
    // We'll create the proper context after getting AV info

    // Make sure temp GL context is current before loading game
    if (tempGLContext && tempGLWindow) {
        if (SDL_GL_MakeCurrent(tempGLWindow, tempGLContext) != 0) {
            std::cerr << "Failed to make temp GL context current: " << SDL_GetError() << std::endl;
        } else {
            std::cout << "Temp GL context is current on this thread" << std::endl;
        }
    }

    // Load the game
    std::cout << "Loading game: " << selectedRomPath << std::endl;
    std::cout.flush();
    
    // Check if this is DK64 (needs special frame dupe handling)
    g_isDK64 = (selectedRomPath.find("Donkey Kong 64") != std::string::npos || 
                selectedRomPath.find("DK64") != std::string::npos ||
                selectedRomPath.find("dk64") != std::string::npos);
    if (g_isDK64) {
        std::cout << "Detected DK64 - enabling frame dupe handling" << std::endl;
    }
    
    // Convert relative path to absolute path if needed
    std::string absoluteRomPath = selectedRomPath;
    if (selectedRomPath.length() > 1 && selectedRomPath[1] != ':') {
        // Relative path - make it absolute using executable directory
        absoluteRomPath = exeDir + "\\" + selectedRomPath;
    }
    
    if (!core.loadGame(absoluteRomPath)) {
        std::cerr << "Failed to load game" << std::endl;
        core.unloadCore();
        SDL_Quit();
        return 1;
    }
    std::cout << "Game loaded successfully" << std::endl;
    std::cout.flush();
    
    // Load SRAM (battery save) if it exists
    std::string sramPath = selectedRomPath;
    size_t lastDot = sramPath.find_last_of('.');
    if (lastDot != std::string::npos) {
        sramPath = sramPath.substr(0, lastDot);
    }
    sramPath += ".srm";
    
    std::cout << "Loading SRAM from: " << sramPath << std::endl;
    core.loadSRAM(sramPath);

    // Get AV info and initialize renderers
    std::cout << "Getting AV info..." << std::endl;
    const auto& avInfo = core.getAVInfo();
    const auto& sysInfo = core.getSystemInfo();
    auto& config = configManager.getConfig();
    
    // Store game resolution globally for viewport calculation
    g_gameWidth = avInfo.geometry.base_width;
    g_gameHeight = avInfo.geometry.base_height;
    
    // Write to log file for debugging
    std::ofstream logFile("pdemu_debug.log", std::ios::app);
    logFile << "=== NEW SESSION ===" << std::endl;
    logFile << "[INIT] Setting g_gameWidth=" << g_gameWidth << ", g_gameHeight=" << g_gameHeight << std::endl;
    logFile << "[INIT] avInfo.geometry.base_width=" << avInfo.geometry.base_width 
            << ", base_height=" << avInfo.geometry.base_height << std::endl;
    logFile << "[INIT] avInfo.geometry.max_width=" << avInfo.geometry.max_width 
            << ", max_height=" << avInfo.geometry.max_height << std::endl;
    logFile.close();
    
    std::cout << "[INIT] Setting g_gameWidth=" << g_gameWidth << ", g_gameHeight=" << g_gameHeight << std::endl;
    std::cout << "[INIT] avInfo.geometry.base_width=" << avInfo.geometry.base_width 
              << ", base_height=" << avInfo.geometry.base_height << std::endl;
    
    std::cout << "Core: " << sysInfo.library_name << " " << sysInfo.library_version << std::endl;
    std::cout << "Resolution: " << avInfo.geometry.base_width << "x" << avInfo.geometry.base_height << std::endl;
    std::cout << "Max Resolution: " << avInfo.geometry.max_width << "x" << avInfo.geometry.max_height << std::endl;
    std::cout << "FPS: " << avInfo.timing.fps << std::endl;

    // Initialize video renderer (OpenGL or SDL2 based on system)
    std::string windowTitle = std::string(sysInfo.library_name) + " - pdEMU";
    SDL_Window* gameWindow = nullptr;
    
    if (g_useOpenGL) {
        // NOTE: Keep temporary context alive until after we create the real window
        // and call context_reset() - the core may need GL during retro_load_game()
        
        // Get desktop display mode to match screen size
        SDL_DisplayMode displayMode;
        if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0) {
            std::cerr << "Failed to get desktop display mode: " << SDL_GetError() << std::endl;
            displayMode.w = 1920;
            displayMode.h = 1080;
        }
        
        std::cout << "Desktop resolution: " << displayMode.w << "x" << displayMode.h << std::endl;
        
        // Create fullscreen window (fills entire screen)
        // The viewport will handle centering 4:3 content with black bars
        int gameWindowWidth = displayMode.w;
        int gameWindowHeight = displayMode.h;
        
        std::cout << "Initializing OpenGL renderer..." << std::endl;
        std::cout << "Creating fullscreen borderless window at " << gameWindowWidth << "x" << gameWindowHeight << std::endl;
        
        // Create borderless fullscreen window
        gameWindow = SDL_CreateWindow(
            windowTitle.c_str(),
            0, 0,  // Top-left corner of screen
            gameWindowWidth,
            gameWindowHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP
        );
        
        if (!gameWindow) {
            std::cerr << "Failed to create OpenGL window: " << SDL_GetError() << std::endl;
            core.unloadGame();
            core.unloadCore();
            SDL_Quit();
            return 1;
        }
        
        // Store window pointer globally for viewport calculation
        g_gameWindow = gameWindow;
        
        // Calculate aspect ratio from core geometry
        float aspectRatio = (float)avInfo.geometry.base_width / avInfo.geometry.base_height;
        
        // Override aspect ratio for systems that should always use 4:3
        if (system && (system->name == "ps1" || system->name == "nes" || 
                      system->name == "snes" || system->name == "gba" ||
                      system->name == "genesis" || system->name == "n64")) {
            aspectRatio = 4.0f / 3.0f;
            std::cout << "Forcing 4:3 aspect ratio for " << system->displayName << std::endl;
        }
        
        if (!videoRendererGL.init(gameWindow, avInfo.geometry.base_width, avInfo.geometry.base_height, aspectRatio)) {
            std::cerr << "Failed to initialize OpenGL renderer" << std::endl;
            SDL_DestroyWindow(gameWindow);
            core.unloadGame();
            core.unloadCore();
            SDL_Quit();
            return 1;
        }
        
        videoRendererGL.setInternalScale(config.internalScale);
        videoRendererGL.setLinearFilter(config.linearFilter);
        
        // Call the core's context_reset callback now that OpenGL context is ready
        // Make sure the context is current before calling the callback
        videoRendererGL.makeCurrent();
        
        // Enable vsync for smooth rendering
        SDL_GL_SetSwapInterval(1);
        std::cout << "VSync enabled (swap interval = " << SDL_GL_GetSwapInterval() << ")" << std::endl;
        
        // Verify GL context is valid before calling context_reset
        std::cout << "Verifying OpenGL context before context_reset..." << std::endl;
        std::cout.flush();
        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);
        const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
        
        std::cout << "GL Vendor: " << (vendor ? (const char*)vendor : "NULL") << std::endl;
        std::cout << "GL Renderer: " << (renderer ? (const char*)renderer : "NULL") << std::endl;
        std::cout << "GL Version: " << (version ? (const char*)version : "NULL") << std::endl;
        std::cout << "GLSL Version: " << (glsl_version ? (const char*)glsl_version : "NULL") << std::endl;
        std::cout.flush();
        
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "GL error before context_reset: 0x" << std::hex << err << std::dec << std::endl;
        }
        
        std::cout << "About to call core.callContextReset()..." << std::endl;
        std::cout.flush();
        core.callContextReset();
        std::cout << "core.callContextReset() completed" << std::endl;
        std::cout.flush();
        
        // NOW we can clean up the temporary context - the real one is active
        if (tempGLContext) {
            std::cout << "Cleaning up temporary GL context..." << std::endl;
            SDL_GL_DeleteContext(tempGLContext);
            tempGLContext = nullptr;
        }
        if (tempGLWindow) {
            SDL_DestroyWindow(tempGLWindow);
            tempGLWindow = nullptr;
        }
    } else {
        // Use SDL2 software renderer
        if (!videoRenderer.init(windowTitle.c_str(), 
                               avInfo.geometry.base_width * config.windowScale, 
                               avInfo.geometry.base_height * config.windowScale)) {
            std::cerr << "Failed to initialize video renderer" << std::endl;
            core.unloadGame();
            core.unloadCore();
            SDL_Quit();
            return 1;
        }
        gameWindow = videoRenderer.getWindow();
    }
    
    // Set pixel format and rendering options (only for SDL2 renderer)
    if (!g_useOpenGL) {
        auto pixelFormat = core.getPixelFormat();
        if (pixelFormat == RETRO_PIXEL_FORMAT_RGB565) {
            videoRenderer.setPixelFormat(PIXEL_FORMAT_RGB565);
        } else if (pixelFormat == RETRO_PIXEL_FORMAT_0RGB1555) {
            videoRenderer.setPixelFormat(PIXEL_FORMAT_0RGB1555);
        } else {
            videoRenderer.setPixelFormat(PIXEL_FORMAT_XRGB8888);
        }
        
        videoRenderer.setInternalScale(config.internalScale);
        videoRenderer.setLinearFilter(config.linearFilter);
    }
    
    std::cout << "Window scale: " << config.windowScale << "x" << std::endl;
    std::cout << "Internal scale: " << config.internalScale << "x" << std::endl;
    std::cout << "Filter: " << (config.linearFilter ? "Linear" : "Nearest") << std::endl;

    // Initialize audio renderer
    if (!audioRenderer.init(avInfo.timing.sample_rate)) {
        std::cerr << "Failed to initialize audio renderer" << std::endl;
        if (g_useOpenGL) {
            videoRendererGL.shutdown();
        } else {
            videoRenderer.shutdown();
        }
        core.unloadGame();
        core.unloadCore();
        SDL_Quit();
        return 1;
    }

    // Re-initialize GUI for in-game menu
    // For OpenGL, we need to create a separate renderer for ImGui or use ImGui OpenGL backend
    // For now, skip GUI in OpenGL mode to avoid conflicts
    if (!g_useOpenGL) {
        if (!guiManager.init(videoRenderer.getWindow(), videoRenderer.getRenderer())) {
            std::cerr << "Failed to re-initialize GUI" << std::endl;
        }
    }

    std::cout << "\nGame started! Press ESC for menu\n" << std::endl;

    // Main emulation loop
    auto frameTime = std::chrono::microseconds(static_cast<long>(1000000.0 / avInfo.timing.fps));
    auto lastTime = std::chrono::high_resolution_clock::now();
    auto fpsTime = lastTime;
    int frameCount = 0;
    float currentFPS = 0.0f;
    bool fastForward = false;
    bool showMenu = false;
    bool showSettingsMenu = false;
    bool firstFrame = true;

    // Set debug mode for OpenGL error skipping/logging
    videoRendererGL.setDebugMode(guiManager.getDebugMode());

    while (!inputHandler.shouldQuit()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        
        // Check for controller exit combo (Start + Select)
        auto controllers = controllerManager.getConnectedControllers();
        for (const auto& ctrl : controllers) {
            if (controllerManager.isExitComboPressed(ctrl.instanceId)) {
                std::cout << "\nController exit combo detected - returning to ROM browser..." << std::endl;
                inputHandler.setQuit(true);
                break;
            }
        }
        
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Handle window close button - this should exit the emulator
            if (event.type == SDL_QUIT) {
                inputHandler.setQuit(true);
                break;
            }
            
            // Let InputHandler handle controller events
            inputHandler.handleEvent(event);
            
            // Only process ImGui events if not using OpenGL (ImGui not initialized for OpenGL)
            if (!g_useOpenGL && !showMenu) {
                guiManager.processEvent(&event);
            }
            
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        // For OpenGL mode, ESC quits directly (no menu support yet)
                        if (g_useOpenGL) {
                            inputHandler.setQuit(true);
                        } else {
                            if (showSettingsMenu) {
                                showSettingsMenu = false;
                            } else {
                                showMenu = !showMenu;
                            }
                        }
                        break;
                    case SDLK_F1:
                        config.showFPS = !config.showFPS;
                        break;
                    case SDLK_F2:
                        if (config.internalScale > 1) {
                            config.internalScale--;
                            if (g_useOpenGL) {
                                videoRendererGL.setInternalScale(config.internalScale);
                            } else {
                                videoRenderer.setInternalScale(config.internalScale);
                            }
                            std::cout << "Internal scale: " << config.internalScale << "x" << std::endl;
                        }
                        break;
                    case SDLK_F3:
                        if (config.internalScale < 4) {
                            config.internalScale++;
                            if (g_useOpenGL) {
                                videoRendererGL.setInternalScale(config.internalScale);
                            } else {
                                videoRenderer.setInternalScale(config.internalScale);
                            }
                            std::cout << "Internal scale: " << config.internalScale << "x" << std::endl;
                        }
                        break;
                    case SDLK_F4:
                        config.linearFilter = !config.linearFilter;
                        if (g_useOpenGL) {
                            videoRendererGL.setLinearFilter(config.linearFilter);
                        } else {
                            videoRenderer.setLinearFilter(config.linearFilter);
                        }
                        std::cout << "Filter: " << (config.linearFilter ? "Linear" : "Nearest") << std::endl;
                        break;
                    case SDLK_F10:
                        if (!g_useOpenGL) {
                            showSettingsMenu = !showSettingsMenu;
                        }
                        break;
                    case SDLK_TAB:
                        fastForward = true;
                        break;
                }
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_TAB) {
                    fastForward = false;
                }
            }
        }

        // Update input (only if not in menu)
        // In OpenGL mode, always update input since we don't have GUI
        if (g_useOpenGL) {
            if (!showMenu) {
                inputHandler.update();
            }
        } else {
            if (!showMenu && !guiManager.wantsCaptureKeyboard()) {
                inputHandler.update();
            }
        }

        // Run one frame (pause if menu is open)
        if (!showMenu) {
            if (g_useOpenGL && firstFrame) {
                std::cout << "About to call core.run() for first time..." << std::endl;
            }
            
            static int runCounter = 0;
            
            core.run();
            
            if (g_useOpenGL && firstFrame) {
                std::cout << "First core.run() completed successfully" << std::endl;
                firstFrame = false;
            }
            
            // Debug: Check if video_refresh was called
            static int debugCounter = 0;
            if (debugCounter++ % 60 == 0) {
                std::cout << "[MAIN] core.run() #" << runCounter << std::endl;
            }
            runCounter++;
        }
        
        // Render based on renderer type
        if (g_useOpenGL) {
            // Only present when core actually rendered a frame (not on dupes/nulls)
            if (g_shouldPresent) {
                videoRendererGL.present();
                g_shouldPresent = false;
            } else if (g_isDK64) {
                // DK64-specific: Frame dupe - still need to maintain timing, so sleep for frame duration
                // This prevents running too fast and desyncing audio
                std::this_thread::sleep_for(std::chrono::microseconds(16666)); // ~60fps
            }
            // For non-DK64 games: don't sleep on null frames, audio timing handles sync
        } else {
            // Render GUI overlay (SDL2 only)
            if (showMenu || showSettingsMenu || config.showFPS) {
                guiManager.beginFrame();
                
                if (showMenu) {
                    guiManager.renderMainMenu(showMenu, showSettingsMenu);
                }
                
                if (showSettingsMenu) {
                    guiManager.renderSettings(config, settingsShouldApply);
                    if (settingsShouldApply) {
                        videoRenderer.setInternalScale(config.internalScale);
                        videoRenderer.setLinearFilter(config.linearFilter);
                        configManager.save();
                        settingsShouldApply = false;
                    }
                }
                
                if (config.showFPS) {
                    guiManager.renderFPSCounter(currentFPS);
                }
                
                guiManager.endFrame(videoRenderer.getRenderer());
            }
            
            videoRenderer.present();
        }
        
        // FPS counter
        frameCount++;
        auto fpsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - fpsTime);
        if (fpsElapsed.count() >= 1000) {
            currentFPS = frameCount * 1000.0f / fpsElapsed.count();
            frameCount = 0;
            fpsTime = currentTime;
        }

        // Frame timing - VSync handles timing for OpenGL, manual sleep for software renderer
        if (!g_useOpenGL && !fastForward && !showMenu) {
            auto frameElapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - currentTime);
            
            // Only sleep if we're more than 500us ahead of schedule
            // This gives audio callbacks priority and reduces hitching
            if (frameElapsed < frameTime) {
                auto sleepTime = frameTime - frameElapsed;
                if (sleepTime.count() > 500) {
                    // Sleep for slightly less than needed to avoid oversleeping
                    std::this_thread::sleep_for(sleepTime - std::chrono::microseconds(100));
                }
            }
        }
        // For OpenGL with VSync, SDL_GL_SwapWindow blocks until vblank, so no manual sleep needed

        lastTime = std::chrono::high_resolution_clock::now();
    }

    // Save config before returning to ROM browser
    configManager.getConfig().lastRomPath = selectedRomPath;
    configManager.save();

    // Cleanup emulator
    std::cout << "Closing emulator, returning to ROM browser..." << std::endl;
    
    // Save SRAM (battery save) before unloading - reuse the sramPath from earlier
    std::cout << "Saving SRAM to: " << sramPath << std::endl;
    core.saveSRAM(sramPath);
    
    // Unload game first
    std::cout << "Unloading game..." << std::endl;
    core.unloadGame();
    
    // Shutdown audio
    std::cout << "Shutting down audio..." << std::endl;
    audioRenderer.shutdown();
    
    if (!g_useOpenGL) {
        guiManager.shutdown();
    }
    
    if (g_useOpenGL) {
        // Make context current before cleanup
        std::cout << "Cleaning up OpenGL context..." << std::endl;
        if (gameWindow && SDL_GL_GetCurrentContext()) {
            videoRendererGL.makeCurrent();
            
            // Call the core's context_destroy callback if it exists
            core.callContextDestroy();
        }
        
        // Shutdown video renderer (this will delete the GL context)
        videoRendererGL.shutdown();
        
        // Destroy window after GL context is gone
        if (gameWindow) {
            std::cout << "Destroying game window..." << std::endl;
            SDL_DestroyWindow(gameWindow);
            gameWindow = nullptr;
            g_gameWindow = nullptr;  // Clear global pointer
        }
    } else {
        videoRenderer.shutdown();
    }
    
    // Unload core last
    std::cout << "Unloading core..." << std::endl;
    core.unloadCore();
    
    // Reset OpenGL flag
    g_useOpenGL = false;

    // Get desktop resolution for fullscreen ROM browser
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        std::cerr << "Failed to get display mode: " << SDL_GetError() << std::endl;
        displayMode.w = 1920;
        displayMode.h = 1080;
    }

    // Recreate fullscreen borderless ROM browser window for next game
    window = SDL_CreateWindow(
        "pdEMU - Universal Emulator",
        0, 0,
        displayMode.w, displayMode.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP
    );

    if (!window) {
        std::cerr << "Failed to recreate window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to recreate renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Reinitialize GUI for ROM browser
    if (!guiManager.init(window, renderer)) {
        std::cerr << "Failed to reinitialize GUI" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // End of main loop iteration - will return to ROM browser
    }

    // Final cleanup when user quits the application
    std::cout << "Shutting down..." << std::endl;
    guiManager.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
