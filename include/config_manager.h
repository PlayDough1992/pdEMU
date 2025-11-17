#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>

struct EmulatorConfig {
    // Video settings
    int windowScale = 3;           // Window scaling factor (1-6)
    int internalScale = 1;         // Internal rendering scale (1-4)
    bool vsync = true;
    bool linearFilter = true;
    
    // Audio settings
    float audioVolume = 1.0f;      // 0.0 - 1.0
    
    // Input settings
    std::map<std::string, int> keyBindings;
    
    // Paths
    std::string lastRomPath;
    std::string coresPath = "cores";
    std::string romsPath = "ROMS";
    std::string biosPath = "BIOS";
    std::string savesPath = "SAVES";
    
    // Other
    bool showFPS = false;
    bool fastForward = false;
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    bool load(const std::string& configPath = "config.json");
    bool save(const std::string& configPath = "config.json");
    
    EmulatorConfig& getConfig() { return m_config; }
    const EmulatorConfig& getConfig() const { return m_config; }
    
    // Store original relative paths to preserve them when saving
    void setOriginalPaths(const std::string& cores, const std::string& roms, 
                         const std::string& bios, const std::string& saves) {
        m_originalCoresPath = cores;
        m_originalRomsPath = roms;
        m_originalBiosPath = bios;
        m_originalSavesPath = saves;
    }
    
private:
    EmulatorConfig m_config;
    std::string m_configPath;
    
    // Store original relative paths
    std::string m_originalCoresPath;
    std::string m_originalRomsPath;
    std::string m_originalBiosPath;
    std::string m_originalSavesPath;
    
    void setDefaults();
};

#endif // CONFIG_MANAGER_H
