#include "core_manager.h"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
#endif
#include <algorithm>
#include <iostream>

CoreManager::CoreManager() {
    initializeCoreMappings();
}

CoreManager::~CoreManager() {
}

void CoreManager::initializeCoreMappings() {
    // Map system names to preferred core files
#ifdef _WIN32
    const char* ext = ".dll";
#else
    const char* ext = ".so";
#endif
    
    m_systemToCoreFile["gba"] = std::string("mgba_libretro") + ext;
    m_systemToCoreFile["gb"] = std::string("gambatte_libretro") + ext;
    m_systemToCoreFile["nes"] = std::string("fceumm_libretro") + ext;
    m_systemToCoreFile["snes"] = std::string("snes9x_libretro") + ext;
    m_systemToCoreFile["n64"] = std::string("mupen64plus_next_libretro") + ext;
    m_systemToCoreFile["nds"] = std::string("desmume_libretro") + ext;
    m_systemToCoreFile["gamecube"] = std::string("dolphin_libretro") + ext;
    m_systemToCoreFile["wii"] = std::string("dolphin_libretro") + ext;
    m_systemToCoreFile["genesis"] = std::string("genesis_plus_gx_libretro") + ext;
    m_systemToCoreFile["sms"] = std::string("genesis_plus_gx_libretro") + ext;
    m_systemToCoreFile["gamegear"] = std::string("genesis_plus_gx_libretro") + ext;
    m_systemToCoreFile["saturn"] = std::string("beetle_saturn_libretro") + ext;
    m_systemToCoreFile["dreamcast"] = std::string("flycast_libretro") + ext;
    m_systemToCoreFile["psx"] = std::string("beetle_psx_hw_libretro") + ext;
    m_systemToCoreFile["ps2"] = std::string("play_libretro") + ext;
    m_systemToCoreFile["psp"] = std::string("ppsspp_libretro") + ext;
    m_systemToCoreFile["arcade"] = std::string("fbneo_libretro") + ext;
    m_systemToCoreFile["atari2600"] = std::string("stella_libretro") + ext;
    m_systemToCoreFile["atari7800"] = std::string("prosystem_libretro") + ext;
    m_systemToCoreFile["pce"] = std::string("beetle_pce_fast_libretro") + ext;
    m_systemToCoreFile["ngp"] = std::string("beetle_ngp_libretro") + ext;
    m_systemToCoreFile["wonderswan"] = std::string("beetle_wswan_libretro") + ext;
    m_systemToCoreFile["dos"] = std::string("dosbox_pure_libretro") + ext;
}

bool CoreManager::scanCoresDirectory(const std::string& coresPath) {
    m_cores.clear();
    
#ifdef _WIN32
    // Windows implementation using FindFirstFile/FindNextFile
    std::string searchPath = coresPath + "\\*.dll";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open cores directory: " << coresPath << std::endl;
        return false;
    }
    
    do {
        // Skip directories
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        
        std::string filename = findData.cFileName;
        
        // Skip if not a libretro core
        if (filename.find("libretro") == std::string::npos) {
            continue;
        }
        
        CoreInfo info;
        info.filename = filename;
        info.fullPath = coresPath + "\\" + filename;
        info.isLoaded = false;
        
        // Extract core name from filename (e.g., "mgba_libretro.dll" -> "mgba")
        size_t pos = filename.find("_libretro");
        if (pos != std::string::npos) {
            info.coreName = filename.substr(0, pos);
        } else {
            info.coreName = filename;
        }
        
        m_cores.push_back(info);
    } while (FindNextFileA(hFind, &findData));
    
    FindClose(hFind);
#else
    // Linux implementation using dirent
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
#endif
    
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
