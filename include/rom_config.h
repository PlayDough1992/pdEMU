#ifndef ROM_CONFIG_H
#define ROM_CONFIG_H

#include <string>
#include <vector>
#include <map>

// Per-ROM configuration
struct RomConfig {
    std::string romPath;
    std::string romName;
    
    // Cheat codes
    struct Cheat {
        std::string name;
        std::string code;
        bool enabled;
    };
    std::vector<Cheat> cheats;
    
    // Custom input mapping (if different from global)
    std::map<std::string, int> customInputMap;
    bool useCustomInput;
    
    // Graphics settings
    int internalScale; // 1-4x, -1 = use global
    bool linearFilter; // -1 = use global
    
    // Audio settings
    float volumeLevel; // 0.0-1.0, -1 = use global
    
    // Core-specific settings
    std::map<std::string, std::string> coreOptions;
    
    // Mods/Hacks
    std::vector<std::string> enabledMods;
    
    // Save state slots
    std::string lastSaveState;
    int quickSaveSlot;
};

class RomConfigManager {
public:
    RomConfigManager();
    ~RomConfigManager();
    
    bool loadRomConfig(const std::string& romPath, RomConfig& config);
    bool saveRomConfig(const RomConfig& config);
    
    // Cheat management
    bool addCheat(RomConfig& config, const std::string& name, const std::string& code);
    bool removeCheat(RomConfig& config, const std::string& name);
    bool toggleCheat(RomConfig& config, const std::string& name);
    
    // Get config file path for a ROM
    std::string getConfigPath(const std::string& romPath);
    
private:
    std::string m_configDir;
    
    std::string getRomBaseName(const std::string& romPath);
    void parseConfigFile(const std::string& filepath, RomConfig& config);
    void writeConfigFile(const std::string& filepath, const RomConfig& config);
};

#endif // ROM_CONFIG_H
