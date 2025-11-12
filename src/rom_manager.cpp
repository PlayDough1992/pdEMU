#include "rom_manager.h"
#include "system_database.h"
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif
#include <algorithm>
#include <iostream>

RomManager::RomManager() : m_systemDb(nullptr) {
}

RomManager::~RomManager() {
}

bool RomManager::isValidRomExtension(const std::string& filename) {
    if (!m_systemDb) {
        // Fallback to basic check
        size_t dotPos = filename.find_last_of('.');
        if (dotPos == std::string::npos) return false;
        
        std::string ext = filename.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return (ext == ".gba" || ext == ".zip" || ext == ".7z");
    }
    
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return false;
    
    std::string ext = filename.substr(dotPos);
    const SystemInfo* system = m_systemDb->getSystemByExtension(ext);
    
    return system != nullptr;
}

std::string RomManager::extractDisplayName(const std::string& filename) {
    // Remove extension
    size_t dotPos = filename.find_last_of('.');
    std::string name = (dotPos != std::string::npos) ? filename.substr(0, dotPos) : filename;
    
    // Remove region tags like (USA), (Europe), etc.
    size_t parenPos = name.find_last_of('(');
    if (parenPos != std::string::npos) {
        name = name.substr(0, parenPos);
    }
    
    // Trim whitespace
    name.erase(0, name.find_first_not_of(" \t"));
    name.erase(name.find_last_not_of(" \t") + 1);
    
    return name;
}

bool RomManager::scanDirectory(const std::string& romsPath) {
    m_romList.clear();
    
#ifdef _WIN32
    // Windows implementation using FindFirstFile/FindNextFile
    std::string searchPath = romsPath + "\\*.*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open ROMs directory: " << romsPath << std::endl;
        return false;
    }
    
    do {
        // Skip directories
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        
        std::string filename = findData.cFileName;
        
        // Skip . and ..
        if (filename[0] == '.') continue;
        
        if (!isValidRomExtension(filename)) continue;
        
        RomInfo info;
        info.filename = filename;
        info.fullPath = romsPath + "\\" + filename;
        info.displayName = extractDisplayName(filename);
        
        // Detect system type
        size_t dotPos = filename.find_last_of('.');
        if (dotPos != std::string::npos && m_systemDb) {
            // First, check for system marker in filename (e.g., "_GCM", "_PS2")
            const SystemInfo* system = m_systemDb->getSystemByMarker(filename);
            
            // If no marker found, fall back to extension detection
            if (!system) {
                std::string ext = filename.substr(dotPos);
                system = m_systemDb->getSystemByExtension(ext);
            }
            
            if (system) {
                info.systemName = system->name;
                info.systemDisplayName = system->displayName;
            } else {
                info.systemName = "unknown";
                info.systemDisplayName = "Unknown System";
            }
        } else {
            info.systemName = "unknown";
            info.systemDisplayName = "Unknown System";
        }
        
        // Get file size (Windows)
        LARGE_INTEGER fileSize;
        fileSize.LowPart = findData.nFileSizeLow;
        fileSize.HighPart = findData.nFileSizeHigh;
        info.fileSize = fileSize.QuadPart;
        info.isValid = true;
        
        m_romList.push_back(info);
    } while (FindNextFileA(hFind, &findData));
    
    FindClose(hFind);
#else
    // Linux implementation using dirent
    DIR* dir = opendir(romsPath.c_str());
    if (!dir) {
        std::cerr << "Failed to open ROMs directory: " << romsPath << std::endl;
        return false;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip . and ..
        if (entry->d_name[0] == '.') continue;
        
        std::string filename = entry->d_name;
        if (!isValidRomExtension(filename)) continue;
        
        RomInfo info;
        info.filename = filename;
        info.fullPath = romsPath + "/" + filename;
        info.displayName = extractDisplayName(filename);
        
        // Detect system type
        size_t dotPos = filename.find_last_of('.');
        if (dotPos != std::string::npos && m_systemDb) {
            // First, check for system marker in filename (e.g., "_GCM", "_PS2")
            const SystemInfo* system = m_systemDb->getSystemByMarker(filename);
            
            // If no marker found, fall back to extension detection
            if (!system) {
                std::string ext = filename.substr(dotPos);
                system = m_systemDb->getSystemByExtension(ext);
            }
            
            if (system) {
                info.systemName = system->name;
                info.systemDisplayName = system->displayName;
            } else {
                info.systemName = "unknown";
                info.systemDisplayName = "Unknown System";
            }
        } else {
            info.systemName = "unknown";
            info.systemDisplayName = "Unknown System";
        }
        
        // Get file size
        struct stat st;
        if (stat(info.fullPath.c_str(), &st) == 0) {
            info.fileSize = st.st_size;
            info.isValid = true;
        } else {
            info.fileSize = 0;
            info.isValid = false;
        }
        
        m_romList.push_back(info);
    }
    
    closedir(dir);
#endif
    
    // Sort alphabetically
    std::sort(m_romList.begin(), m_romList.end(), 
        [](const RomInfo& a, const RomInfo& b) {
            if (a.systemName != b.systemName) {
                return a.systemName < b.systemName;
            }
            return a.displayName < b.displayName;
        });
    
    std::cout << "Found " << m_romList.size() << " ROMs" << std::endl;
    return true;
}

std::map<std::string, std::vector<RomInfo>> RomManager::getRomsBySystem() const {
    std::map<std::string, std::vector<RomInfo>> romsBySystem;
    
    for (const auto& rom : m_romList) {
        romsBySystem[rom.systemName].push_back(rom);
    }
    
    return romsBySystem;
}
