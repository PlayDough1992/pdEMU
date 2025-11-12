#ifndef CHEAT_DATABASE_H
#define CHEAT_DATABASE_H

#include <string>
#include <vector>
#include <map>

struct CheatCode {
    std::string name;
    std::string description;
    std::string code; // In format: "address:value" or "XXXXXXXX YYYY"
    std::string system; // "gba", "nes", "snes", etc.
};

class CheatDatabase {
public:
    CheatDatabase();
    
    // Get cheats for a specific ROM by name or CRC
    std::vector<CheatCode> getCheatsForRom(const std::string& romName, const std::string& system);
    
    // Search cheats by keyword
    std::vector<CheatCode> searchCheats(const std::string& query, const std::string& system = "");
    
    // Add custom cheat
    bool addCustomCheat(const CheatCode& cheat);
    
    // Load cheats from database file
    bool loadDatabase(const std::string& filepath);
    bool saveDatabase(const std::string& filepath);
    
private:
    // Game name -> cheats mapping
    std::map<std::string, std::vector<CheatCode>> m_cheats;
    
    void initializeBuiltInCheats();
    std::string normalizeRomName(const std::string& romName);
};

#endif // CHEAT_DATABASE_H
