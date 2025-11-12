#include "config_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ConfigManager::ConfigManager() {
    setDefaults();
}

ConfigManager::~ConfigManager() {
}

void ConfigManager::setDefaults() {
    m_config.windowScale = 3;
    m_config.internalScale = 1;
    m_config.vsync = true;
    m_config.linearFilter = true;
    m_config.audioVolume = 1.0f;
    m_config.coresPath = "cores";
    m_config.romsPath = "ROMS";
    m_config.biosPath = "BIOS";
    m_config.savesPath = "SAVES";
    m_config.showFPS = false;
    m_config.fastForward = false;
}

bool ConfigManager::load(const std::string& configPath) {
    m_configPath = configPath;
    std::ifstream file(configPath);
    
    if (!file.is_open()) {
        std::cout << "Config file not found, using defaults" << std::endl;
        return false;
    }
    
    // Simple key=value parser
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Remove whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Parse values
        if (key == "window_scale") m_config.windowScale = std::stoi(value);
        else if (key == "internal_scale") m_config.internalScale = std::stoi(value);
        else if (key == "vsync") m_config.vsync = (value == "true" || value == "1");
        else if (key == "linear_filter") m_config.linearFilter = (value == "true" || value == "1");
        else if (key == "audio_volume") m_config.audioVolume = std::stof(value);
        else if (key == "cores_path") m_config.coresPath = value;
        else if (key == "roms_path") m_config.romsPath = value;
        else if (key == "bios_path") m_config.biosPath = value;
        else if (key == "saves_path") m_config.savesPath = value;
        else if (key == "show_fps") m_config.showFPS = (value == "true" || value == "1");
        else if (key == "last_rom") m_config.lastRomPath = value;
    }
    
    file.close();
    std::cout << "Config loaded from " << configPath << std::endl;
    return true;
}

bool ConfigManager::save(const std::string& configPath) {
    std::ofstream file(configPath.empty() ? m_configPath : configPath);
    
    if (!file.is_open()) {
        std::cerr << "Failed to save config to " << configPath << std::endl;
        return false;
    }

    file << "# pdEMU Configuration\n\n";
    file << "# Video Settings\n";
    file << "window_scale=" << m_config.windowScale << "\n";
    file << "internal_scale=" << m_config.internalScale << "\n";
    file << "vsync=" << (m_config.vsync ? "true" : "false") << "\n";
    file << "linear_filter=" << (m_config.linearFilter ? "true" : "false") << "\n\n";
    
    file << "# Audio Settings\n";
    file << "audio_volume=" << m_config.audioVolume << "\n\n";
    
    file << "# Paths\n";
    file << "cores_path=" << m_config.coresPath << "\n";
    file << "roms_path=" << m_config.romsPath << "\n";
    file << "bios_path=" << m_config.biosPath << "\n";
    file << "saves_path=" << m_config.savesPath << "\n\n";
    
    file << "# Other\n";
    file << "show_fps=" << (m_config.showFPS ? "true" : "false") << "\n";
    file << "last_rom=" << m_config.lastRomPath << "\n";
    
    file.close();
    std::cout << "Config saved to " << (configPath.empty() ? m_configPath : configPath) << std::endl;
    return true;
}
