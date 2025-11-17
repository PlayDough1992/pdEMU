#include "rom_config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#endif

RomConfigManager::RomConfigManager() : m_configDir("SAVES/configs") {
    // Create config directory if it doesn't exist
    struct stat st = {0};
    if (stat("SAVES", &st) == -1) {
        mkdir("SAVES", 0700);
    }
    if (stat(m_configDir.c_str(), &st) == -1) {
        mkdir(m_configDir.c_str(), 0700);
    }
}

RomConfigManager::~RomConfigManager() {
}

std::string RomConfigManager::getRomBaseName(const std::string& romPath) {
    // Extract filename without extension
    size_t lastSlash = romPath.find_last_of("/\\");
    size_t lastDot = romPath.find_last_of('.');
    
    std::string filename;
    if (lastSlash != std::string::npos) {
        filename = romPath.substr(lastSlash + 1);
    } else {
        filename = romPath;
    }
    
    if (lastDot != std::string::npos && lastDot > lastSlash) {
        filename = filename.substr(0, lastDot - (lastSlash + 1));
    }
    
    return filename;
}

std::string RomConfigManager::getConfigPath(const std::string& romPath) {
    std::string baseName = getRomBaseName(romPath);
    return m_configDir + "/" + baseName + ".cfg";
}

bool RomConfigManager::loadRomConfig(const std::string& romPath, RomConfig& config) {
    std::string configPath = getConfigPath(romPath);
    
    // Initialize with defaults
    config.romPath = romPath;
    config.romName = getRomBaseName(romPath);
    config.useCustomInput = false;
    config.internalScale = -1;
    config.linearFilter = false;
    config.volumeLevel = -1.0f;
    config.quickSaveSlot = 0;
    config.cheats.clear();
    config.customInputMap.clear();
    config.coreOptions.clear();
    config.enabledMods.clear();
    
    // Try to load existing config
    std::ifstream file(configPath);
    if (!file.is_open()) {
        // No config exists yet, return defaults
        return true;
    }
    
    parseConfigFile(configPath, config);
    return true;
}

void RomConfigManager::parseConfigFile(const std::string& filepath, RomConfig& config) {
    std::ifstream file(filepath);
    std::string line;
    std::string currentSection = "";
    
    while (std::getline(file, line)) {
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty()) continue;
        
        // Check for section headers
        if (line[0] == '[' && line[line.length()-1] == ']') {
            currentSection = line.substr(1, line.length()-2);
            continue;
        }
        
        // Parse key=value pairs
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        
        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        
        // Trim key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (currentSection == "graphics") {
            if (key == "internal_scale") config.internalScale = std::stoi(value);
            else if (key == "linear_filter") config.linearFilter = (value == "true" || value == "1");
        }
        else if (currentSection == "audio") {
            if (key == "volume") config.volumeLevel = std::stof(value);
        }
        else if (currentSection.find("cheat") == 0) {
            // Cheats are stored as [cheat_NAME]
            size_t underscorePos = currentSection.find('_');
            if (underscorePos != std::string::npos) {
                std::string cheatName = currentSection.substr(underscorePos + 1);
                
                // Find or create cheat
                RomConfig::Cheat* cheat = nullptr;
                for (auto& c : config.cheats) {
                    if (c.name == cheatName) {
                        cheat = &c;
                        break;
                    }
                }
                
                if (!cheat) {
                    config.cheats.push_back({cheatName, "", false});
                    cheat = &config.cheats.back();
                }
                
                if (key == "code") cheat->code = value;
                else if (key == "enabled") cheat->enabled = (value == "true" || value == "1");
            }
        }
        else if (currentSection == "core_options") {
            config.coreOptions[key] = value;
        }
        else if (currentSection == "mods") {
            if (key == "enabled") {
                config.enabledMods.push_back(value);
            }
        }
        else if (currentSection == "save_states") {
            if (key == "quick_slot") config.quickSaveSlot = std::stoi(value);
            else if (key == "last_state") config.lastSaveState = value;
        }
    }
}

bool RomConfigManager::saveRomConfig(const RomConfig& config) {
    std::string configPath = getConfigPath(config.romPath);
    writeConfigFile(configPath, config);
    return true;
}

void RomConfigManager::writeConfigFile(const std::string& filepath, const RomConfig& config) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to write config: " << filepath << std::endl;
        return;
    }
    
    file << "# ROM Configuration for: " << config.romName << "\n\n";
    
    // Graphics settings
    file << "[graphics]\n";
    file << "internal_scale=" << config.internalScale << "\n";
    file << "linear_filter=" << (config.linearFilter ? "true" : "false") << "\n\n";
    
    // Audio settings
    file << "[audio]\n";
    file << "volume=" << config.volumeLevel << "\n\n";
    
    // Cheats
    for (const auto& cheat : config.cheats) {
        file << "[cheat_" << cheat.name << "]\n";
        file << "code=" << cheat.code << "\n";
        file << "enabled=" << (cheat.enabled ? "true" : "false") << "\n\n";
    }
    
    // Core options
    if (!config.coreOptions.empty()) {
        file << "[core_options]\n";
        for (const auto& opt : config.coreOptions) {
            file << opt.first << "=" << opt.second << "\n";
        }
        file << "\n";
    }
    
    // Mods
    if (!config.enabledMods.empty()) {
        file << "[mods]\n";
        for (const auto& mod : config.enabledMods) {
            file << "enabled=" << mod << "\n";
        }
        file << "\n";
    }
    
    // Save states
    file << "[save_states]\n";
    file << "quick_slot=" << config.quickSaveSlot << "\n";
    file << "last_state=" << config.lastSaveState << "\n";
    
    file.close();
}

bool RomConfigManager::addCheat(RomConfig& config, const std::string& name, const std::string& code) {
    // Check if cheat already exists
    for (const auto& cheat : config.cheats) {
        if (cheat.name == name) {
            return false; // Already exists
        }
    }
    
    config.cheats.push_back({name, code, false});
    return true;
}

bool RomConfigManager::removeCheat(RomConfig& config, const std::string& name) {
    auto it = std::remove_if(config.cheats.begin(), config.cheats.end(),
        [&name](const RomConfig::Cheat& cheat) { return cheat.name == name; });
    
    if (it != config.cheats.end()) {
        config.cheats.erase(it, config.cheats.end());
        return true;
    }
    
    return false;
}

bool RomConfigManager::toggleCheat(RomConfig& config, const std::string& name) {
    for (auto& cheat : config.cheats) {
        if (cheat.name == name) {
            cheat.enabled = !cheat.enabled;
            return true;
        }
    }
    return false;
}
