#include "core_manager.h"
#include <dirent.h>
#include <algorithm>
#include <iostream>

CoreManager::CoreManager() {
    initializeCoreMappings();
}

CoreManager::~CoreManager() {
}

void CoreManager::initializeCoreMappings() {
    // Map system names to preferred core files
    m_systemToCoreFile["gba"] = "mgba_libretro.so";
    m_systemToCoreFile["gb"] = "gambatte_libretro.so";
    m_systemToCoreFile["nes"] = "fceumm_libretro.so";
    m_systemToCoreFile["snes"] = "snes9x_libretro.so";
    m_systemToCoreFile["n64"] = "mupen64plus_next_libretro.so";
    m_systemToCoreFile["nds"] = "desmume_libretro.so";
    m_systemToCoreFile["gamecube"] = "dolphin_libretro.so";
    m_systemToCoreFile["wii"] = "dolphin_libretro.so";
    m_systemToCoreFile["genesis"] = "genesis_plus_gx_libretro.so";
    m_systemToCoreFile["sms"] = "genesis_plus_gx_libretro.so";
    m_systemToCoreFile["gamegear"] = "genesis_plus_gx_libretro.so";
    m_systemToCoreFile["saturn"] = "beetle_saturn_libretro.so";
    m_systemToCoreFile["dreamcast"] = "flycast_libretro.so";
    m_systemToCoreFile["psx"] = "beetle_psx_hw_libretro.so";
    m_systemToCoreFile["ps2"] = "play_libretro.so";
    m_systemToCoreFile["psp"] = "ppsspp_libretro.so";
    m_systemToCoreFile["arcade"] = "fbneo_libretro.so";
    m_systemToCoreFile["atari2600"] = "stella_libretro.so";
    m_systemToCoreFile["atari7800"] = "prosystem_libretro.so";
    m_systemToCoreFile["pce"] = "beetle_pce_fast_libretro.so";
    m_systemToCoreFile["ngp"] = "beetle_ngp_libretro.so";
    m_systemToCoreFile["wonderswan"] = "beetle_wswan_libretro.so";
    m_systemToCoreFile["dos"] = "dosbox_pure_libretro.so";
}

bool CoreManager::scanCoresDirectory(const std::string& coresPath) {
    m_cores.clear();
    
    DIR* dir = opendir(coresPath.c_str());
    if (!dir) {
        std::cerr << "Failed to open cores directory: " << coresPath << std::endl;
        return false;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        
        // Check if it's a libretro core (.so file)
        if (filename.length() < 3 || filename.substr(filename.length() - 3) != ".so") {
            continue;
        }
        
        // Skip if not a libretro core
        if (filename.find("libretro") == std::string::npos) {
            continue;
        }
        
        CoreInfo info;
        info.filename = filename;
        info.fullPath = coresPath + "/" + filename;
        info.isLoaded = false;
        
        // Extract core name from filename (e.g., "mgba_libretro.so" -> "mgba")
        size_t pos = filename.find("_libretro");
        if (pos != std::string::npos) {
            info.coreName = filename.substr(0, pos);
        } else {
            info.coreName = filename;
        }
        
        m_cores.push_back(info);
    }
    
    closedir(dir);
    
    // Sort cores by name
    std::sort(m_cores.begin(), m_cores.end(),
        [](const CoreInfo& a, const CoreInfo& b) {
            return a.coreName < b.coreName;
        });
    
    std::cout << "Found " << m_cores.size() << " libretro cores" << std::endl;
    return true;
}

std::string CoreManager::getBestCoreForSystem(const std::string& systemName) const {
    auto it = m_systemToCoreFile.find(systemName);
    if (it != m_systemToCoreFile.end()) {
        // Check if the preferred core exists
        for (const auto& core : m_cores) {
            if (core.filename == it->second) {
                return core.fullPath;
            }
        }
    }
    
    // Try to find any matching core based on system name
    for (const auto& core : m_cores) {
        if (core.coreName.find(systemName) != std::string::npos) {
            return core.fullPath;
        }
    }
    
    return "";
}

const CoreInfo* CoreManager::findCoreForSystem(const std::string& systemName) const {
    auto it = m_systemToCoreFile.find(systemName);
    if (it != m_systemToCoreFile.end()) {
        for (const auto& core : m_cores) {
            if (core.filename == it->second) {
                return &core;
            }
        }
    }
    return nullptr;
}

const CoreInfo* CoreManager::findCoreForExtension(const std::string& extension) const {
    // This would need system database integration
    // For now, return nullptr
    return nullptr;
}
