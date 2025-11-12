#ifndef ROM_MANAGER_H
#define ROM_MANAGER_H

#include <string>
#include <vector>
#include <map>

struct RomInfo {
    std::string filename;
    std::string fullPath;
    std::string displayName;
    std::string systemName;        // "gba", "nes", "snes", etc.
    std::string systemDisplayName; // "Game Boy Advance", "Nintendo Entertainment System"
    size_t fileSize;
    bool isValid;
};

class SystemDatabase;

class RomManager {
public:
    RomManager();
    ~RomManager();
    
    void setSystemDatabase(SystemDatabase* db) { m_systemDb = db; }
    SystemDatabase* getSystemDatabase() const { return m_systemDb; }
    
    bool scanDirectory(const std::string& romsPath);
    const std::vector<RomInfo>& getRomList() const { return m_romList; }
    std::map<std::string, std::vector<RomInfo>> getRomsBySystem() const;
    
    std::string extractDisplayName(const std::string& filename);
    bool isValidRomExtension(const std::string& filename);
    
private:
    std::vector<RomInfo> m_romList;
    SystemDatabase* m_systemDb;
};

#endif // ROM_MANAGER_H
