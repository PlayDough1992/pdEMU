#include "libretro_core.h"
#include <SDL2/SDL.h>
#include <GL/gl.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdarg>
#include <set>
#include <map>
#include <string>

// Global core instance for environment callback
static LibretroCore* g_coreInstance = nullptr;

// Global variable store for core options (now with external linkage)
std::map<std::string, std::string> g_coreVariables;

// External globals from main.cpp for viewport calculation
extern SDL_Window* g_gameWindow;
extern unsigned g_gameWidth;
extern unsigned g_gameHeight;

// Hardware rendering callbacks (must be C functions with C linkage)
extern "C" {
    // Logging callback for cores
    static void core_log_printf(enum retro_log_level level, const char* fmt, ...) {
        const char* level_str = "";
        switch (level) {
            case RETRO_LOG_DEBUG: level_str = "[DEBUG]"; break;
            case RETRO_LOG_INFO: level_str = "[INFO]"; break;
            case RETRO_LOG_WARN: level_str = "[WARN]"; break;
            case RETRO_LOG_ERROR: level_str = "[ERROR]"; break;
            default: level_str = "[UNKNOWN]"; break;
        }
        
        va_list args;
        va_start(args, fmt);
        printf("%s ", level_str);
        vprintf(fmt, args);
        va_end(args);
        fflush(stdout);
    }
    
    static retro_proc_address_t hw_get_proc_address(const char* sym) {
        if (!sym) {
            return nullptr;
        }
        
        retro_proc_address_t addr = (retro_proc_address_t)SDL_GL_GetProcAddress(sym);
        return addr;
    }

    static uintptr_t hw_get_current_framebuffer() {
        // This is called every frame by the core before it renders
        // Set up viewport for proper aspect ratio
        
        if (g_gameWindow) {
            int windowWidth, windowHeight;
            SDL_GetWindowSize(g_gameWindow, &windowWidth, &windowHeight);
            
            // No aspect ratio correction - just fill the entire window
            int viewportWidth = windowWidth;
            int viewportHeight = windowHeight;
            int viewportX = 0;
            int viewportY = 0;
            
            // Print viewport info once
            static bool viewportPrinted = false;
            if (!viewportPrinted && g_coreInstance) {
                std::cout << "[pdEMU] " << g_coreInstance->getSystemInfo().library_name << " Viewport Position: " 
                         << viewportX << ", " << viewportY << ", " 
                         << viewportWidth << ", " << viewportHeight << std::endl;
                viewportPrinted = true;
            }
            
            // Clear the entire window to black
            glViewport(0, 0, windowWidth, windowHeight);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            // Set viewport for the game to render into (full window)
            glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
        }
        
        // Return 0 for default framebuffer (render directly to window backbuffer)
        return (uintptr_t)0;
    }
}

LibretroCore::LibretroCore()
    : m_coreHandle(nullptr)
    , m_coreLoaded(false)
    , m_gameLoaded(false)
    , m_usesHardwareRender(false)
    , m_pixelFormat(RETRO_PIXEL_FORMAT_XRGB8888)
    , m_systemDir("./BIOS")
    , m_saveDir("./SAVES")
    , m_hw_context_reset(nullptr)
    , m_hw_context_destroy(nullptr)
{
    memset(&m_systemInfo, 0, sizeof(m_systemInfo));
    memset(&m_avInfo, 0, sizeof(m_avInfo));
}

LibretroCore::~LibretroCore() {
    unloadGame();
    unloadCore();
}

bool LibretroCore::loadCore(const std::string& corePath) {
    if (m_coreLoaded) {
        std::cerr << "Core already loaded" << std::endl;
        return false;
    }

    // Load the shared library
#ifdef _WIN32
    m_coreHandle = LoadLibraryA(corePath.c_str());
    if (!m_coreHandle) {
        std::cerr << "Failed to load core: Error code " << GetLastError() << std::endl;
        return false;
    }
#else
    m_coreHandle = dlopen(corePath.c_str(), RTLD_LAZY);
    if (!m_coreHandle) {
        std::cerr << "Failed to load core: " << dlerror() << std::endl;
        return false;
    }
#endif

    // Load all core functions
    if (!loadCoreFunctions()) {
#ifdef _WIN32
        FreeLibrary((HMODULE)m_coreHandle);
#else
        dlclose(m_coreHandle);
#endif
        m_coreHandle = nullptr;
        return false;
    }

    // Set global instance for callbacks
    g_coreInstance = this;

    // Set default core variables for mupen64plus resolution
    // The core will request these via RETRO_ENVIRONMENT_GET_VARIABLE
    // Note: Setting both to same resolution - mupen64plus will handle 4:3 centering internally
    g_coreVariables["mupen64plus-169screensize"] = "1920x1080";  // Window resolution
    g_coreVariables["mupen64plus-43screensize"] = "1920x1080";   // Window resolution (mupen64plus handles 4:3 aspect)
    g_coreVariables["mupen64plus-framerate"] = "fullspeed";
    g_coreVariables["mupen64plus-BilinearMode"] = "standard";
    
    // N64 video settings to fix flickering (especially DK64)
    g_coreVariables["mupen64plus-FrameDuping"] = "false";  // Disable frame duping
    g_coreVariables["mupen64plus-EnableFBEmulation"] = "true";  // Enable framebuffer emulation (fixes DK64 flickering)
    g_coreVariables["mupen64plus-EnableCopyColorToRDRAM"] = "sync";  // Sync color buffer copies (more accurate)
    g_coreVariables["mupen64plus-EnableCopyDepthToRDRAM"] = "software";  // Software depth buffer copies
    g_coreVariables["mupen64plus-EnableCopyAuxToRDRAM"] = "false";  // Disable aux buffer copies
    g_coreVariables["mupen64plus-EnableN64DepthCompare"] = "false";  // Disable N64 depth compare (can cause flicker)
    g_coreVariables["mupen64plus-EnableLegacyBlending"] = "false";  // Use accurate blending
    g_coreVariables["mupen64plus-EnableHWLighting"] = "false";  // Disable hardware lighting (can cause issues)
    g_coreVariables["mupen64plus-CorrectTexrectCoords"] = "auto";  // Auto-correct texrect coords
    g_coreVariables["mupen64plus-txFilterMode"] = "none";  // Disable texture filtering that might cause flicker
    g_coreVariables["mupen64plus-ThreadedRenderer"] = "false";  // Disable threaded rendering (can cause timing issues)
    
    // PS1 core audio settings to reduce latency
    g_coreVariables["pcsx_rearmed_spu_reverb"] = "disabled";  // Disable reverb for less latency
    g_coreVariables["pcsx_rearmed_spu_interpolation"] = "simple";  // Simple interpolation
    g_coreVariables["pcsx_rearmed_async_cd"] = "sync";  // Sync CD access

    // Set environment callback
    m_retro_set_environment(environmentCallback);

    // Initialize the core
    m_retro_init();

    // Get system info
    m_retro_get_system_info(&m_systemInfo);

    m_coreLoaded = true;
    return true;
}

bool LibretroCore::loadCoreFunctions() {
#ifdef _WIN32
    #define LOAD_SYM(name) \
        m_##name = (name##_t)GetProcAddress((HMODULE)m_coreHandle, #name); \
        if (!m_##name) { \
            std::cerr << "Failed to load symbol: " #name << std::endl; \
            return false; \
        }
#else
    #define LOAD_SYM(name) \
        m_##name = (name##_t)dlsym(m_coreHandle, #name); \
        if (!m_##name) { \
            std::cerr << "Failed to load symbol: " #name << std::endl; \
            return false; \
        }
#endif

    LOAD_SYM(retro_init)
    LOAD_SYM(retro_deinit)
    LOAD_SYM(retro_api_version)
    LOAD_SYM(retro_get_system_info)
    LOAD_SYM(retro_get_system_av_info)
    LOAD_SYM(retro_set_environment)
    LOAD_SYM(retro_set_video_refresh)
    LOAD_SYM(retro_set_audio_sample)
    LOAD_SYM(retro_set_audio_sample_batch)
    LOAD_SYM(retro_set_input_poll)
    LOAD_SYM(retro_set_input_state)
    LOAD_SYM(retro_set_controller_port_device)
    LOAD_SYM(retro_reset)
    LOAD_SYM(retro_run)
    LOAD_SYM(retro_serialize_size)
    LOAD_SYM(retro_serialize)
    LOAD_SYM(retro_unserialize)
    LOAD_SYM(retro_load_game)
    LOAD_SYM(retro_unload_game)
    LOAD_SYM(retro_get_region)
    LOAD_SYM(retro_get_memory_data)
    LOAD_SYM(retro_get_memory_size)

    #undef LOAD_SYM

    return true;
}

bool LibretroCore::loadGame(const std::string& gamePath) {
    if (!m_coreLoaded) {
        std::cerr << "Core not loaded" << std::endl;
        return false;
    }

    if (m_gameLoaded) {
        std::cerr << "Game already loaded" << std::endl;
        return false;
    }

    // For large disc-based games (PS2, GameCube, etc.), don't load entire file into memory
    // Check if core needs full data or just path
    bool needFullData = m_systemInfo.need_fullpath == false;
    
    std::vector<uint8_t> gameData;
    size_t fileSize = 0;
    
    if (needFullData) {
        // Read the game file into memory (for small ROMs)
        std::ifstream file(gamePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open game file: " << gamePath << std::endl;
            return false;
        }

        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        gameData.resize(fileSize);
        if (!file.read(reinterpret_cast<char*>(gameData.data()), fileSize)) {
            std::cerr << "Failed to read game file" << std::endl;
            return false;
        }
        file.close();
    } else {
        // Just verify file exists (for large disc images)
        std::ifstream file(gamePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open game file: " << gamePath << std::endl;
            return false;
        }
        fileSize = file.tellg();
        file.close();
    }

    // Prepare game info
    retro_game_info gameInfo;
    gameInfo.path = gamePath.c_str();
    gameInfo.data = needFullData ? gameData.data() : nullptr;
    gameInfo.size = fileSize;
    gameInfo.meta = nullptr;

    // Load the game



    std::cout.flush();
    
    if (!m_retro_load_game(&gameInfo)) {
        std::cerr << "Core failed to load game" << std::endl;
        return false;
    }

    // Get AV info
    m_retro_get_system_av_info(&m_avInfo);

    // Set controller port device
    m_retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);

    m_gameLoaded = true;
    return true;
}

void LibretroCore::unloadGame() {
    if (m_gameLoaded && m_retro_unload_game) {
        m_retro_unload_game();
        m_gameLoaded = false;
    }
}

void LibretroCore::unloadCore() {
    if (m_coreLoaded) {
        if (m_retro_deinit) {
            m_retro_deinit();
        }
        if (m_coreHandle) {
#ifdef _WIN32
            FreeLibrary((HMODULE)m_coreHandle);
#else
            dlclose(m_coreHandle);
#endif
            m_coreHandle = nullptr;
        }
        m_coreLoaded = false;
        g_coreInstance = nullptr;
        
        // Clear hardware render callbacks
        m_hw_context_reset = nullptr;
        m_hw_context_destroy = nullptr;
    }
}

void LibretroCore::reset() {
    if (m_gameLoaded && m_retro_reset) {
        m_retro_reset();
    }
}

void LibretroCore::run() {
    if (m_gameLoaded && m_retro_run) {
        m_retro_run();
    }
}

bool LibretroCore::saveState(const std::string& path) {
    if (!m_gameLoaded) return false;

    size_t size = m_retro_serialize_size();
    if (size == 0) return false;

    std::vector<uint8_t> data(size);
    if (!m_retro_serialize(data.data(), size)) {
        return false;
    }

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(data.data()), size);
    return file.good();
}

bool LibretroCore::loadState(const std::string& path) {
    if (!m_gameLoaded) return false;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    return m_retro_unserialize(data.data(), size);
}

void* LibretroCore::getMemoryData(unsigned type) {
    if (m_gameLoaded && m_retro_get_memory_data) {
        return m_retro_get_memory_data(type);
    }
    return nullptr;
}

size_t LibretroCore::getMemorySize(unsigned type) {
    if (m_gameLoaded && m_retro_get_memory_size) {
        return m_retro_get_memory_size(type);
    }
    return 0;
}

bool LibretroCore::saveSRAM(const std::string& path) {
    if (!m_gameLoaded) {
        return false;
    }
    
    // Get SRAM data from core
    void* data = getMemoryData(RETRO_MEMORY_SAVE_RAM);
    size_t size = getMemorySize(RETRO_MEMORY_SAVE_RAM);
    
    if (!data || size == 0) {
        // No SRAM to save (some games don't use it)
        return true;
    }
    
    // Write SRAM to file
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open SRAM file for writing: " << path << std::endl;
        return false;
    }
    
    file.write(static_cast<const char*>(data), size);
    file.close();
    
    if (file.fail()) {
        std::cerr << "Failed to write SRAM data to: " << path << std::endl;
        return false;
    }

    return true;
}

bool LibretroCore::loadSRAM(const std::string& path) {
    if (!m_gameLoaded) {
        return false;
    }
    
    // Check if SRAM file exists
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // No SRAM file exists yet (first time playing)
        return true;
    }
    
    // Get file size
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Get SRAM buffer from core
    void* data = getMemoryData(RETRO_MEMORY_SAVE_RAM);
    size_t size = getMemorySize(RETRO_MEMORY_SAVE_RAM);
    
    if (!data || size == 0) {
        std::cerr << "Core doesn't support SRAM" << std::endl;
        return false;
    }
    
    if (fileSize != size) {
        std::cerr << "SRAM file size mismatch: expected " << size << ", got " << fileSize << std::endl;
        return false;
    }
    
    // Read SRAM from file into core's memory
    file.read(static_cast<char*>(data), size);
    file.close();
    
    if (file.fail()) {
        std::cerr << "Failed to read SRAM data from: " << path << std::endl;
        return false;
    }

    return true;
}

void LibretroCore::setVideoRefreshCallback(retro_video_refresh_t callback) {
    if (m_retro_set_video_refresh) {
        m_retro_set_video_refresh(callback);
    }
}

void LibretroCore::setAudioSampleCallback(retro_audio_sample_t callback) {
    if (m_retro_set_audio_sample) {
        m_retro_set_audio_sample(callback);
    }
}

void LibretroCore::setAudioSampleBatchCallback(retro_audio_sample_batch_t callback) {
    if (m_retro_set_audio_sample_batch) {
        m_retro_set_audio_sample_batch(callback);
    }
}

void LibretroCore::setInputPollCallback(retro_input_poll_t callback) {
    if (m_retro_set_input_poll) {
        m_retro_set_input_poll(callback);
    }
}

void LibretroCore::setInputStateCallback(retro_input_state_t callback) {
    if (m_retro_set_input_state) {
        m_retro_set_input_state(callback);
    }
}

bool LibretroCore::environmentCallback(unsigned cmd, void* data) {
    if (!g_coreInstance) return false;

    // Log all environment calls for debugging
    // std::cout << "Environment call: " << cmd << std::endl;

    switch (cmd) {
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
            const char** dir = (const char**)data;
            *dir = g_coreInstance->m_systemDir.c_str();

            return true;
        }
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
            const char** dir = (const char**)data;
            *dir = g_coreInstance->m_saveDir.c_str();

            return true;
        }
        case 30: { // RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY
            const char** dir = (const char**)data;
            *dir = g_coreInstance->m_systemDir.c_str();  // Use same as system dir

            return true;
        }
        case 15: { // RETRO_ENVIRONMENT_GET_VARIABLE
            // Core is requesting a variable value
            struct retro_variable {
                const char* key;
                const char* value;
            };
            retro_variable* var = (retro_variable*)data;
            if (var && var->key) {
                auto it = g_coreVariables.find(var->key);
                if (it != g_coreVariables.end()) {
                    var->value = it->second.c_str();
                    return true;
                }
            }
            // Variable not found - return false
            return false;
        }
        case 16: { // RETRO_ENVIRONMENT_SET_VARIABLES
            // Core is providing list of available variables with their default values
            struct retro_variable {
                const char* key;
                const char* value;
            };
            const retro_variable* vars = (const retro_variable*)data;
            if (vars) {
                // Parse and store variables, but prefer frontend-set values
                for (int i = 0; vars[i].key != nullptr; i++) {
                    std::string key = vars[i].key;
                    if (g_coreVariables.count(key)) {
                        // Already set by frontend, do not overwrite
                        continue;
                    }
                    if (vars[i].value) {
                        // Parse "description; value1|value2|..." format
                        std::string value_str = vars[i].value;
                        size_t semicolon = value_str.find(';');
                        if (semicolon != std::string::npos) {
                            // Extract default value (first option after semicolon)
                            std::string options = value_str.substr(semicolon + 1);
                            size_t pipe = options.find('|');
                            std::string default_val = (pipe != std::string::npos) 
                                ? options.substr(0, pipe) 
                                : options;
                            // Trim whitespace
                            default_val.erase(0, default_val.find_first_not_of(" \t"));
                            default_val.erase(default_val.find_last_not_of(" \t") + 1);
                            g_coreVariables[key] = default_val;
                        }
                    }
                }
            }
            return true;
        }
        case 17: { // RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE
            // Core asking if variables changed
            bool* updated = (bool*)data;
            if (updated) {
                *updated = false;
            }
            return true;
        }
        case 27: { // RETRO_ENVIRONMENT_GET_LOG_INTERFACE
            // Core wants logging interface - provide it!
            retro_log_callback* log_cb = (retro_log_callback*)data;
            if (log_cb) {
                log_cb->log = core_log_printf;
                return true;
            }
            return false;
        }
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
            const enum retro_pixel_format* fmt = (const enum retro_pixel_format*)data;
            g_coreInstance->m_pixelFormat = *fmt;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_CAN_DUPE: {
            bool* can_dupe = (bool*)data;
            *can_dupe = true;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
            return false;
        }
        case 58: { // RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER
            // Core is asking which hardware renderer we prefer
            unsigned* preferred = (unsigned*)data;
            if (preferred) {
                *preferred = RETRO_HW_CONTEXT_OPENGL_CORE; // We support OpenGL Core
            }
            return true;
        }
        case 14: { // RETRO_ENVIRONMENT_SET_HW_RENDER
            // Hardware rendering requested - provide OpenGL context

            // Mark that this core uses hardware rendering
            if (g_coreInstance) {
                g_coreInstance->m_usesHardwareRender = true;
            }
            
            // Use the official struct from libretro.h
            retro_hw_render_callback* hw = (retro_hw_render_callback*)data;



            // Log what the core is requesting




            // Log the callback pointers the core provided




            // FIRST: Frontend provides these callbacks that the core will use during initialization



            hw->get_proc_address = hw_get_proc_address;
            hw->get_current_framebuffer = hw_get_current_framebuffer;
            
            // IMPORTANT: Initialize bool fields that may not be set by the core
            // According to libretro.h comments, some fields like cache_context should be set by frontend
            if (hw->cache_context == 0) {
                hw->cache_context = false;  // Don't try to preserve context across resets
            }
            if (hw->debug_context == 0) {
                hw->debug_context = false;  // No debug context
            }
            // Set bottom_left_origin to true for OpenGL coordinate system
            hw->bottom_left_origin = true;



            // Verify the struct content right before calling context_reset




            // SECOND: Store the callbacks for later
            if (g_coreInstance) {
                if (hw->context_reset) {

                    g_coreInstance->m_hw_context_reset = hw->context_reset;
                }
                if (hw->context_destroy) {

                    g_coreInstance->m_hw_context_destroy = hw->context_destroy;
                }
                
                // DON'T call context_reset here - will be called after creating real window
                // The temporary context is just for loading, the real context is for rendering

            }
            
            // NOTE: Do NOT call context_reset again later - it's already been called

            std::cout.flush(); // Make sure output is written before returning
            
            return true;
        }
        default:
            // Log unhandled environment calls
            static std::set<unsigned> logged_calls;
            if (logged_calls.find(cmd) == logged_calls.end()) {

                logged_calls.insert(cmd);
            }
            return false;
    }
}

void LibretroCore::callContextReset() {
    if (m_hw_context_reset) {


        m_hw_context_reset();

    } else {

    }
}

void LibretroCore::callContextDestroy() {
    if (m_hw_context_destroy && m_coreLoaded) {

        try {
            m_hw_context_destroy();

        } catch (...) {
            std::cerr << "Exception in context_destroy callback" << std::endl;
        }
    } else {

    }
}
