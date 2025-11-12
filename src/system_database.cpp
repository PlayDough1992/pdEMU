#include "system_database.h"
#include <algorithm>

SystemDatabase::SystemDatabase() {
}

SystemDatabase::~SystemDatabase() {
}

void SystemDatabase::addSystem(const SystemInfo& info) {
    m_systems[info.name] = info;
    
    // Map all extensions to this system
    for (const auto& ext : info.extensions) {
        std::string lowerExt = ext;
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
        m_extensionToSystem[lowerExt] = info.name;
    }
}

void SystemDatabase::initialize() {
    // Nintendo Game Boy Advance
    addSystem({
        "gba",
        "Game Boy Advance",
        {".gba", ".agb"},
        {"mgba_libretro.so", "vbam_libretro.so", "gpsp_libretro.so"},
        "Nintendo",
        2001,
        false,
        "gba_bios.bin",
        "_GBA"
    });
    
    // Nintendo Game Boy / Game Boy Color
    addSystem({
        "gb",
        "Game Boy / Game Boy Color",
        {".gb", ".gbc", ".sgb"},
        {"gambatte_libretro.so", "mgba_libretro.so", "sameboy_libretro.so"},
        "Nintendo",
        1989,
        false,
        "gb_bios.bin",
        "_GB"
    });
    
    // Nintendo Entertainment System
    addSystem({
        "nes",
        "Nintendo Entertainment System",
        {".nes", ".fds", ".unf", ".unif"},
        {"fceumm_libretro.so", "nestopia_libretro.so", "mesen_libretro.so"},
        "Nintendo",
        1983,
        false,
        "",
        "_NES"
    });
    
    // Super Nintendo Entertainment System
    addSystem({
        "snes",
        "Super Nintendo Entertainment System",
        {".smc", ".sfc", ".swc", ".fig"},
        {"snes9x_libretro.so", "bsnes_libretro.so", "bsnes_mercury_balanced_libretro.so"},
        "Nintendo",
        1990,
        false,
        "",
        "_SNES"
    });
    
    // Nintendo 64
    addSystem({
        "n64",
        "Nintendo 64",
        {".n64", ".z64", ".v64"},
        {"mupen64plus_next_libretro.so", "parallel_n64_libretro.so"},
        "Nintendo",
        1996,
        false,
        "",
        "_N64"
    });
    
    // Nintendo DS
    addSystem({
        "nds",
        "Nintendo DS",
        {".nds", ".bin"},
        {"desmume_libretro.so", "melonds_libretro.so"},
        "Nintendo",
        2004,
        true,
        "bios7.bin",
        "_NDS"
    });
    
    // Nintendo GameCube
    addSystem({
        "gamecube",
        "Nintendo GameCube",
        {".gcm", ".gcz", ".ciso", ".dol", ".elf", ".rvz", ".nkit.iso"},
        {"dolphin_libretro.so"},
        "Nintendo",
        2001,
        true,  // Requires OpenGL hardware rendering
        "",
        "_GCM"
    });
    
    // Nintendo Wii
    addSystem({
        "wii",
        "Nintendo Wii",
        {".wbfs", ".wad", ".rvz", ".nkit.iso"},
        {"dolphin_libretro.so"},
        "Nintendo",
        2006,
        false,
        "",
        "_WII"
    });
    
    // Sega Genesis / Mega Drive
    addSystem({
        "genesis",
        "Sega Genesis / Mega Drive",
        {".md", ".gen", ".smd", ".bin", ".cue", ".iso"},
        {"genesis_plus_gx_libretro.so", "picodrive_libretro.so"},
        "Sega",
        1988,
        false,
        "",
        "_GEN"
    });
    
    // Sega Master System
    addSystem({
        "sms",
        "Sega Master System",
        {".sms"},
        {"genesis_plus_gx_libretro.so", "picodrive_libretro.so"},
        "Sega",
        1985,
        false,
        "",
        "_SMS"
    });
    
    // Sega Game Gear
    addSystem({
        "gamegear",
        "Sega Game Gear",
        {".gg"},
        {"genesis_plus_gx_libretro.so", "gearsystem_libretro.so"},
        "Sega",
        1990,
        false,
        "",
        "_GG"
    });
    
    // Sega Saturn
    addSystem({
        "saturn",
        "Sega Saturn",
        {".cue", ".ccd", ".chd", ".toc", ".m3u"},
        {"beetle_saturn_libretro.so", "yabause_libretro.so"},
        "Sega",
        1994,
        true,
        "saturn_bios.bin",
        "_SAT"
    });
    
    // Sega Dreamcast
    addSystem({
        "dreamcast",
        "Sega Dreamcast",
        {".cdi", ".chd", ".gdi"},
        {"flycast_libretro.so", "redream_libretro.so"},
        "Sega",
        1998,
        true,
        "dc_bios.bin",
        "_DC"
    });
    
    // Sony PlayStation
    addSystem({
        "psx",
        "Sony PlayStation",
        {".cue", ".toc", ".m3u", ".ccd", ".exe", ".pbp", ".chd"},
        {"beetle_psx_hw_libretro.so", "beetle_psx_libretro.so", "pcsx_rearmed_libretro.so"},
        "Sony",
        1994,
        true,
        "scph5501.bin",
        "_PS1"
    });
    
    // Sony PlayStation 2
    addSystem({
        "ps2",
        "Sony PlayStation 2",
        {".iso", ".cso", ".chd", ".bin"},
        {"play_libretro.so"},
        "Sony",
        2000,
        true,
        "SCPH10000.bin",
        "_PS2"
    });
    
    // Sony PSP
    addSystem({
        "psp",
        "Sony PlayStation Portable",
        {".iso", ".cso", ".pbp", ".elf"},
        {"ppsspp_libretro.so"},
        "Sony",
        2004,
        false,
        "",
        "_PSP"
    });
    
    // Arcade
    addSystem({
        "arcade",
        "Arcade",
        {".zip", ".7z"},
        {"mame_libretro.so", "fbneo_libretro.so", "fbalpha2012_libretro.so"},
        "Various",
        1970,
        false,
        "",
        "_ARC"
    });
    
    // Atari 2600
    addSystem({
        "atari2600",
        "Atari 2600",
        {".a26", ".bin"},
        {"stella_libretro.so"},
        "Atari",
        1977,
        false,
        "",
        "_A26"
    });
    
    // Atari 7800
    addSystem({
        "atari7800",
        "Atari 7800",
        {".a78", ".bin"},
        {"prosystem_libretro.so"},
        "Atari",
        1986,
        false,
        "",
        "_A78"
    });
    
    // PC Engine / TurboGrafx-16
    addSystem({
        "pce",
        "PC Engine / TurboGrafx-16",
        {".pce", ".sgx", ".cue", ".ccd", ".chd"},
        {"beetle_pce_fast_libretro.so", "beetle_pce_libretro.so"},
        "NEC",
        1987,
        false,
        "",
        "_PCE"
    });
    
    // Neo Geo Pocket
    addSystem({
        "ngp",
        "Neo Geo Pocket / Color",
        {".ngp", ".ngc"},
        {"beetle_ngp_libretro.so"},
        "SNK",
        1998,
        false,
        "",
        "_NGP"
    });
    
    // WonderSwan
    addSystem({
        "wonderswan",
        "WonderSwan / WonderSwan Color",
        {".ws", ".wsc"},
        {"beetle_wswan_libretro.so"},
        "Bandai",
        1999,
        false,
        "",
        "_WS"
    });
    
    // DOS
    addSystem({
        "dos",
        "MS-DOS",
        {".exe", ".com", ".bat", ".conf"},
        {"dosbox_pure_libretro.so", "dosbox_core_libretro.so"},
        "Microsoft",
        1981,
        false,
        "",
        "_DOS"
    });
}

const SystemInfo* SystemDatabase::getSystemByExtension(const std::string& extension) const {
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    
    auto it = m_extensionToSystem.find(lowerExt);
    if (it != m_extensionToSystem.end()) {
        return &m_systems.at(it->second);
    }
    return nullptr;
}

const SystemInfo* SystemDatabase::getSystemByName(const std::string& name) const {
    auto it = m_systems.find(name);
    if (it != m_systems.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> SystemDatabase::getAllSystemNames() const {
    std::vector<std::string> names;
    for (const auto& pair : m_systems) {
        names.push_back(pair.first);
    }
    return names;
}

const SystemInfo* SystemDatabase::getSystemByMarker(const std::string& filename) const {
    // Check if filename contains a system marker (e.g., "_GCM", "_PS2")
    // Marker should appear before the first period in the filename
    for (const auto& pair : m_systems) {
        const SystemInfo& system = pair.second;
        if (!system.marker.empty()) {
            // Look for marker before the first period
            size_t dotPos = filename.find('.');
            if (dotPos != std::string::npos) {
                std::string nameBeforeExt = filename.substr(0, dotPos);
                // Check if marker is at the end of the base filename
                if (nameBeforeExt.length() >= system.marker.length()) {
                    size_t markerPos = nameBeforeExt.length() - system.marker.length();
                    if (nameBeforeExt.substr(markerPos) == system.marker) {
                        return &system;
                    }
                }
            }
        }
    }
    return nullptr;
}
