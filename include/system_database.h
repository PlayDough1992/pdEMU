#ifndef SYSTEM_DATABASE_H
#define SYSTEM_DATABASE_H

#include <string>
#include <vector>
#include <map>

struct SystemInfo {
    std::string name;
    std::string displayName;
    std::vector<std::string> extensions;
    std::vector<std::string> coreFiles;      // Preferred cores in order
    std::string manufacturer;
    int year;
    bool requiresBios;
    std::string biosFile;
    std::string marker;  // e.g., "_GCM", "_WII", "_PS2" for manual system override
};

class SystemDatabase {
public:
    SystemDatabase();
    ~SystemDatabase();
    
    void initialize();
    
    const SystemInfo* getSystemByExtension(const std::string& extension) const;
    const SystemInfo* getSystemByName(const std::string& name) const;
    const SystemInfo* getSystemByMarker(const std::string& filename) const;
    
    std::vector<std::string> getAllSystemNames() const;
    const std::map<std::string, SystemInfo>& getAllSystems() const { return m_systems; }
    
private:
    std::map<std::string, SystemInfo> m_systems;
    std::map<std::string, std::string> m_extensionToSystem;  // .gba -> "gba"
    
    void addSystem(const SystemInfo& info);
};

#endif // SYSTEM_DATABASE_H
