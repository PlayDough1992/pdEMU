#ifndef CORE_MANAGER_H
#define CORE_MANAGER_H

#include <string>
#include <vector>
#include <map>

struct CoreInfo {
    std::string filename;
    std::string fullPath;
    std::string coreName;
    std::string coreVersion;
    std::vector<std::string> supportedExtensions;
    bool isLoaded;
};

class CoreManager {
public:
    CoreManager();
    ~CoreManager();
    
    bool scanCoresDirectory(const std::string& coresPath);
    
    const CoreInfo* findCoreForSystem(const std::string& systemName) const;
    const CoreInfo* findCoreForExtension(const std::string& extension) const;
    std::string getBestCoreForSystem(const std::string& systemName) const;
    
    const std::vector<CoreInfo>& getAllCores() const { return m_cores; }
    
private:
    std::vector<CoreInfo> m_cores;
    std::map<std::string, std::string> m_systemToCoreFile;  // "gba" -> "mgba_libretro.so"
    
    void initializeCoreMappings();
    bool loadCoreInfo(const std::string& corePath, CoreInfo& info);
};

#endif // CORE_MANAGER_H
