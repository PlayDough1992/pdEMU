#include <map>
#include <string>

// Global core variable store for frontend/core option communication
extern std::map<std::string, std::string> g_coreVariables;
#ifndef LIBRETRO_CORE_H
#define LIBRETRO_CORE_H

#include "libretro.h"
#include <string>
#include <vector>

// Function pointer typedefs for dynamically loaded core functions
typedef void (*retro_init_t)(void);
typedef void (*retro_deinit_t)(void);
typedef unsigned (*retro_api_version_t)(void);
typedef void (*retro_get_system_info_t)(struct retro_system_info *info);
typedef void (*retro_get_system_av_info_t)(struct retro_system_av_info *info);
typedef void (*retro_set_environment_t)(retro_environment_t);
typedef void (*retro_set_video_refresh_t)(retro_video_refresh_t);
typedef void (*retro_set_audio_sample_t)(retro_audio_sample_t);
typedef void (*retro_set_audio_sample_batch_t)(retro_audio_sample_batch_t);
typedef void (*retro_set_input_poll_t)(retro_input_poll_t);
typedef void (*retro_set_input_state_t)(retro_input_state_t);
typedef void (*retro_set_controller_port_device_t)(unsigned port, unsigned device);
typedef void (*retro_reset_t)(void);
typedef void (*retro_run_t)(void);
typedef size_t (*retro_serialize_size_t)(void);
typedef bool (*retro_serialize_t)(void *data, size_t size);
typedef bool (*retro_unserialize_t)(const void *data, size_t size);
typedef bool (*retro_load_game_t)(const struct retro_game_info *game);
typedef void (*retro_unload_game_t)(void);
typedef unsigned (*retro_get_region_t)(void);
typedef void* (*retro_get_memory_data_t)(unsigned id);
typedef size_t (*retro_get_memory_size_t)(unsigned id);

class LibretroCore {
public:
    LibretroCore();
    ~LibretroCore();

    // Core management
    bool loadCore(const std::string& corePath);
    bool loadGame(const std::string& gamePath);
    void unloadGame();
    void unloadCore();
    void reset();
    void run();

    // Get core information
    const retro_system_info& getSystemInfo() const { return m_systemInfo; }
    const retro_system_av_info& getAVInfo() const { return m_avInfo; }
    retro_pixel_format getPixelFormat() const { return m_pixelFormat; }
    
    // Save states
    bool saveState(const std::string& path);
    bool loadState(const std::string& path);
    
    // SRAM (battery save) management
    bool saveSRAM(const std::string& path);
    bool loadSRAM(const std::string& path);
    
    // Memory access
    void* getMemoryData(unsigned type);
    size_t getMemorySize(unsigned type);
    
    // Callback setters
    void setVideoRefreshCallback(retro_video_refresh_t callback);
    void setAudioSampleCallback(retro_audio_sample_t callback);
    void setAudioSampleBatchCallback(retro_audio_sample_batch_t callback);
    void setInputPollCallback(retro_input_poll_t callback);
    void setInputStateCallback(retro_input_state_t callback);
    
    // Hardware rendering
    void callContextReset();
    void callContextDestroy();
    
    bool isGameLoaded() const { return m_gameLoaded; }
    bool isCoreLoaded() const { return m_coreLoaded; }
    bool usesHardwareRender() const { return m_usesHardwareRender; }

private:
    // Core function pointers
    void* m_coreHandle;
    retro_init_t m_retro_init;
    retro_deinit_t m_retro_deinit;
    retro_api_version_t m_retro_api_version;
    retro_get_system_info_t m_retro_get_system_info;
    retro_get_system_av_info_t m_retro_get_system_av_info;
    retro_set_environment_t m_retro_set_environment;
    retro_set_video_refresh_t m_retro_set_video_refresh;
    retro_set_audio_sample_t m_retro_set_audio_sample;
    retro_set_audio_sample_batch_t m_retro_set_audio_sample_batch;
    retro_set_input_poll_t m_retro_set_input_poll;
    retro_set_input_state_t m_retro_set_input_state;
    retro_set_controller_port_device_t m_retro_set_controller_port_device;
    retro_reset_t m_retro_reset;
    retro_run_t m_retro_run;
    retro_serialize_size_t m_retro_serialize_size;
    retro_serialize_t m_retro_serialize;
    retro_unserialize_t m_retro_unserialize;
    retro_load_game_t m_retro_load_game;
    retro_unload_game_t m_retro_unload_game;
    retro_get_region_t m_retro_get_region;
    retro_get_memory_data_t m_retro_get_memory_data;
    retro_get_memory_size_t m_retro_get_memory_size;

    // Core state
    bool m_coreLoaded;
    bool m_gameLoaded;
    bool m_usesHardwareRender;
    retro_system_info m_systemInfo;
    retro_system_av_info m_avInfo;
    retro_pixel_format m_pixelFormat;
    
    // Directories
    std::string m_systemDir;
    std::string m_saveDir;
    
    // Hardware rendering
    void (*m_hw_context_reset)();
    void (*m_hw_context_destroy)();

    // Helper functions
    bool loadCoreFunctions();
    static bool environmentCallback(unsigned cmd, void* data);
};

#endif // LIBRETRO_CORE_H
