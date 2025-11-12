#include "cheat_database.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

CheatDatabase::CheatDatabase() {
    initializeBuiltInCheats();
}

std::string CheatDatabase::normalizeRomName(const std::string& romName) {
    std::string normalized = romName;
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
        [](unsigned char c){ return std::tolower(c); });
    
    // Remove common suffixes
    size_t pos;
    if ((pos = normalized.find(" (usa)")) != std::string::npos) normalized = normalized.substr(0, pos);
    if ((pos = normalized.find(" (europe)")) != std::string::npos) normalized = normalized.substr(0, pos);
    if ((pos = normalized.find(" (japan)")) != std::string::npos) normalized = normalized.substr(0, pos);
    if ((pos = normalized.find(" (world)")) != std::string::npos) normalized = normalized.substr(0, pos);
    if ((pos = normalized.find(" [!]")) != std::string::npos) normalized = normalized.substr(0, pos);
    
    return normalized;
}

void CheatDatabase::initializeBuiltInCheats() {
    // === GAME BOY ADVANCE (mgba, vbam) ===
    
    // Pokemon FireRed/LeafGreen cheats (GBA)
    m_cheats["pokemon firered"] = {
        {"Infinite Money", "Max money (999999)", "82025838 0098967F", "gba"},
        {"Master Ball x99", "99 Master Balls in PC", "82025840 0001", "gba"},
        {"Rare Candy x99", "99 Rare Candies in bag", "82025BD0 0044", "gba"},
        {"Walk Through Walls", "Walk anywhere", "509197D3 542975F4\n78DA95DF 44018CB4", "gba"},
        {"No Random Battles", "No wild Pokemon", "D41DD0CA 33A629E5", "gba"},
    };
    
    m_cheats["pokemon leafgreen"] = m_cheats["pokemon firered"];
    
    // Pokemon Emerald cheats (GBA)
    m_cheats["pokemon emerald"] = {
        {"Infinite Money", "Max money", "82025838 0098967F", "gba"},
        {"Master Ball x99", "99 Master Balls", "82025840 0001", "gba"},
        {"Rare Candy x99", "99 Rare Candies", "82025BD0 0044", "gba"},
        {"Walk Through Walls", "No collision", "7881A409 E2026E0C\n8E883EFF 92E9660D", "gba"},
        {"Shiny Pokemon", "All wild Pokemon are shiny", "39584B19 91B58AB7", "gba"},
    };
    
    // === GAME BOY / GAME BOY COLOR (gambatte, sameboy) ===
    
    // Pokemon Red/Blue (GB)
    m_cheats["pokemon red"] = {
        {"Infinite Money", "Max money", "0199 423F\n019A 000F", "gb"},
        {"Infinite Master Balls", "Master Ball in slot 1", "01D31E 01", "gb"},
        {"Infinite Rare Candies", "Rare Candy in slot 2", "01D320 20", "gb"},
        {"Walk Through Walls", "No collision", "010CD5 C3", "gb"},
    };
    
    m_cheats["pokemon blue"] = m_cheats["pokemon red"];
    
    // Pokemon Gold/Silver (GBC)
    m_cheats["pokemon gold"] = {
        {"Max Money", "999999 money", "01D84E 423F\n01D84F 000F", "gb"},
        {"Master Ball x99", "99 Master Balls", "01D892 63", "gb"},
        {"Rare Candy x99", "99 Rare Candies", "01D8D5 63", "gb"},
        {"All Badges", "8 badges", "01D857 FF", "gb"},
    };
    
    m_cheats["pokemon silver"] = m_cheats["pokemon gold"];
    m_cheats["pokemon crystal"] = m_cheats["pokemon gold"];
    
    // The Legend of Zelda: Link's Awakening (GB)
    m_cheats["link awakening"] = {
        {"Infinite Health", "Full hearts", "01DB5B 18", "gb"},
        {"Infinite Rupees", "Max money", "01DB5D E7\n01DB5E 03", "gb"},
        {"All Items", "Full inventory", "01DB40 FF", "gb"},
    };
    
    m_cheats["zelda link awakening"] = m_cheats["link awakening"];
    
    // Super Mario Land (GB)
    m_cheats["super mario land"] = {
        {"Infinite Lives", "99 lives", "0100A0 63", "gb"},
        {"Invincibility", "Cannot be hurt", "01FF9A 01", "gb"},
    };
    
    // Super Mario Land 2 (GB)
    m_cheats["super mario land 2"] = {
        {"Infinite Lives", "99 lives", "01FF9B 63", "gb"},
        {"Max Coins", "999 coins", "01FF9C E7\n01FF9D 03", "gb"},
    };
    
    // === NES (fceumm, nestopia) ===
    
    // Super Mario Bros (NES)
    m_cheats["super mario bros"] = {
        {"Infinite Lives", "Never lose lives", "075A 09", "nes"},
        {"Invincibility", "Always star power", "079F 01", "nes"},
        {"Always Fire Mario", "Keep fire flowers", "0756 01", "nes"},
    };
    
    // Super Mario Bros 3 (NES)
    m_cheats["super mario bros 3"] = {
        {"Infinite Lives", "99 lives", "0036 63", "nes"},
        {"Infinite Power-ups", "99 items", "05F0 63", "nes"},
        {"Always Tanooki Suit", "Keep tanooki", "0756 0F", "nes"},
        {"Moon Jump", "Hold A to float", "001D 08", "nes"},
    };
    
    // The Legend of Zelda (NES)
    m_cheats["legend of zelda"] = {
        {"Infinite Health", "Full hearts", "0066F 10", "nes"},
        {"Infinite Rupees", "Max money", "066D FF", "nes"},
        {"Infinite Bombs", "99 bombs", "0658 63", "nes"},
        {"All Items", "Full inventory", "0656 FF", "nes"},
    };
    
    // Mega Man 2 (NES)
    m_cheats["mega man 2"] = {
        {"Infinite Health", "Full energy", "06C0 1C", "nes"},
        {"Infinite Lives", "99 lives", "00A8 63", "nes"},
        {"Infinite Weapon Energy", "Never run out", "006B 1C", "nes"},
    };
    
    // Contra (NES)
    m_cheats["contra"] = {
        {"Infinite Lives", "Never lose lives", "0032 63", "nes"},
        {"Invincibility", "Cannot be hurt", "0057 01", "nes"},
    };
    
    // Metroid (NES)
    m_cheats["metroid"] = {
        {"Infinite Health", "Full energy", "0106 C9", "nes"},
        {"Infinite Missiles", "999 missiles", "0109 E7", "nes"},
    };
    
    // === SNES (snes9x, bsnes) ===
    
    // The Legend of Zelda: A Link to the Past (SNES)
    m_cheats["zelda"] = {
        {"Infinite Health", "Hearts never decrease", "7E0373 14", "snes"},
        {"Infinite Rupees", "Max rupees", "7E0360 E7\n7E0361 03", "snes"},
        {"All Items", "Full inventory", "7E0340 FF", "snes"},
    };
    
    // Super Mario World (SNES)
    m_cheats["super mario world"] = {
        {"Infinite Lives", "Never lose lives", "7E0DBF 99", "snes"},
        {"Always Fire Mario", "Keep fire power", "7E0019 02", "snes"},
        {"Always Have Cape", "Keep cape", "7E0019 03", "snes"},
        {"Invincibility", "Cannot be hurt", "7E0071 09", "snes"},
    };
    
    // Sonic the Hedgehog (Genesis)
    m_cheats["sonic"] = {
        {"Infinite Rings", "999 rings", "FFB04C:03E7", "genesis"},
        {"Infinite Lives", "99 lives", "FE12 63", "genesis"},
        {"Invincibility", "Cannot be hurt", "FE2008 01", "genesis"},
    };
    
    // === GENESIS / MEGA DRIVE (genesis_plus_gx, picodrive) ===
    
    // Sonic the Hedgehog 2 (Genesis)
    m_cheats["sonic 2"] = {
        {"Infinite Rings", "999 rings", "FFB04C:03E7", "genesis"},
        {"Infinite Lives", "99 lives", "FE12 63", "genesis"},
        {"Debug Mode", "Access debug features", "FFB001 01", "genesis"},
    };
    
    m_cheats["sonic the hedgehog 2"] = m_cheats["sonic 2"];
    
    // Streets of Rage 2 (Genesis)
    m_cheats["streets of rage 2"] = {
        {"Infinite Health P1", "Player 1 full health", "FFFE02 78", "genesis"},
        {"Infinite Lives P1", "99 lives", "FFFEC0 63", "genesis"},
        {"Invincibility", "Cannot be hurt", "FFFE00 01", "genesis"},
    };
    
    // Mortal Kombat (Genesis)
    m_cheats["mortal kombat"] = {
        {"Infinite Health P1", "Full health bar", "FFEE00 A6", "genesis"},
        {"One Hit Kills", "Instant fatality", "FFEE10 00", "genesis"},
    };
    
    // Phantasy Star IV (Genesis)
    m_cheats["phantasy star iv"] = {
        {"Max Meseta", "999999 money", "FF0100 423F", "genesis"},
        {"All Items", "Full inventory", "FF0200 FFFF", "genesis"},
    };
    
    // === PLAYSTATION (pcsx_rearmed, beetle_psx) ===
    
    // Castlevania: Symphony of the Night (PSX)
    m_cheats["castlevania"] = {
        {"Infinite Health", "HP never decreases", "800A4D38 03E7", "psx"},
        {"Max Gold", "999999 gold", "800973CC 423F\n800973CE 000F", "psx"},
        {"All Items", "Full inventory", "80097964 6363", "psx"},
    };
    
    m_cheats["castlevania symphony"] = m_cheats["castlevania"];
    
    // Final Fantasy VII (PSX)
    m_cheats["final fantasy vii"] = {
        {"Max Gil", "9999999 gil", "8009D260 967F\n8009D262 0098", "psx"},
        {"Infinite HP All", "Party full HP", "8009D26C E703", "psx"},
        {"Infinite MP All", "Party full MP", "8009D270 E703", "psx"},
        {"Max Level All", "Level 99 party", "8009D274 0063", "psx"},
    };
    
    m_cheats["final fantasy 7"] = m_cheats["final fantasy vii"];
    
    // Crash Bandicoot (PSX)
    m_cheats["crash bandicoot"] = {
        {"Infinite Lives", "99 lives", "8008A200 0063", "psx"},
        {"Infinite Aku Aku", "Invincibility mask", "8008A204 0002", "psx"},
        {"Max Wumpa Fruit", "99 fruits", "8008A208 0063", "psx"},
    };
    
    // Spyro the Dragon (PSX)
    m_cheats["spyro"] = {
        {"Infinite Health", "Full HP", "800760C0 0003", "psx"},
        {"Max Gems", "All gems collected", "800760C4 03E7", "psx"},
        {"Infinite Lives", "99 lives", "800760C8 0063", "psx"},
    };
    
    m_cheats["spyro the dragon"] = m_cheats["spyro"];
    
    // Tony Hawk's Pro Skater 2 (PSX)
    m_cheats["tony hawk 2"] = {
        {"Infinite Balance", "Never fall", "80090100 4000", "psx"},
        {"Max Score", "99999999 points", "80090104 05F5\n80090106 00", "psx"},
        {"Unlock All Levels", "Every level available", "80090110 FFFF", "psx"},
    };
    
    m_cheats["tony hawks pro skater 2"] = m_cheats["tony hawk 2"];
    
    // === NINTENDO 64 (mupen64plus_next, parallel_n64) ===
    
    // Super Mario 64 (N64)
    m_cheats["super mario 64"] = {
        {"Infinite Health", "Full health", "8033B21E 0800", "n64"},
        {"Infinite Lives", "99 lives", "8033B21D 0063", "n64"},
        {"Have All Stars", "120 stars", "8033B218 0078", "n64"},
        {"Moon Jump", "Hold L to float", "8033B1B4 4220", "n64"},
    };
    
    // The Legend of Zelda: Ocarina of Time (N64)
    m_cheats["ocarina of time"] = {
        {"Infinite Health", "Full hearts", "8011A5FF 0140", "n64"},
        {"Infinite Rupees", "Max money", "8011A5E0 01C2", "n64"},
        {"All Items", "Full inventory", "8011A644 FFFF", "n64"},
        {"All Masks", "Every mask", "8011A650 FFFF", "n64"},
    };
    
    m_cheats["zelda ocarina"] = m_cheats["ocarina of time"];
    
    // GoldenEye 007 (N64)
    m_cheats["goldeneye"] = {
        {"Infinite Health", "Never die", "80073E10 0064", "n64"},
        {"Infinite Ammo", "Never reload", "80073E14 0063", "n64"},
        {"All Weapons", "Full arsenal", "80073E18 FFFF", "n64"},
        {"Unlock All Levels", "Every mission", "80073E20 FFFF", "n64"},
    };
    
    m_cheats["goldeneye 007"] = m_cheats["goldeneye"];
    
    // Mario Kart 64 (N64)
    m_cheats["mario kart 64"] = {
        {"Always First Place", "Win every race", "80165100 0001", "n64"},
        {"Infinite Turbo", "Always boosting", "80165104 0001", "n64"},
        {"Infinite Items", "Items never run out", "80165108 0001", "n64"},
    };
    
    // === DREAMCAST (flycast, redream) ===
    
    // Sonic Adventure (Dreamcast)
    m_cheats["sonic adventure"] = {
        {"Infinite Rings", "999 rings", "0C120000 03E7", "dreamcast"},
        {"Infinite Lives", "99 lives", "0C120004 0063", "dreamcast"},
        {"Max Score", "999999 points", "0C120008 423F", "dreamcast"},
    };
    
    // Crazy Taxi (Dreamcast)
    m_cheats["crazy taxi"] = {
        {"Infinite Time", "Timer never runs out", "0C150000 4000", "dreamcast"},
        {"Max Cash", "99999 dollars", "0C150004 869F", "dreamcast"},
    };
    
    // === PSP (ppsspp) ===
    
    // God of War: Chains of Olympus (PSP)
    m_cheats["god of war chains"] = {
        {"Infinite Health", "Full HP", "0x08900000 0x447A0000", "psp"},
        {"Infinite Magic", "Full MP", "0x08900004 0x447A0000", "psp"},
        {"Max Red Orbs", "999999 orbs", "0x08900008 0x000F423F", "psp"},
    };
    
    // Grand Theft Auto: Liberty City Stories (PSP)
    m_cheats["gta liberty city"] = {
        {"Infinite Health", "Never die", "0x08910000 0x447A0000", "psp"},
        {"Max Money", "9999999 dollars", "0x08910004 0x0098967F", "psp"},
        {"Infinite Ammo", "Never reload", "0x08910008 0x000003E7", "psp"},
    };
    
    // === ARCADE (fbneo, mame) ===
    
    // Street Fighter II (Arcade)
    m_cheats["street fighter ii"] = {
        {"Infinite Health P1", "Full health bar", "FF8400 B0", "arcade"},
        {"Infinite Time", "Timer frozen", "FF8500 0063", "arcade"},
        {"One Hit Wins", "Instant knockout", "FF8600 00", "arcade"},
    };
    
    m_cheats["street fighter 2"] = m_cheats["street fighter ii"];
    
    // Metal Slug (Arcade)
    m_cheats["metal slug"] = {
        {"Infinite Credits", "99 credits", "106000 0063", "arcade"},
        {"Infinite Health", "Never die", "106004 0003", "arcade"},
        {"Infinite Ammo", "Never reload", "106008 03E7", "arcade"},
        {"Infinite Grenades", "99 bombs", "10600C 0063", "arcade"},
    };
    
    // === ATARI 2600 (stella) ===
    
    // Pitfall (Atari 2600)
    m_cheats["pitfall"] = {
        {"Infinite Lives", "Never lose", "00D5 63", "atari2600"},
        {"Max Score", "999999 points", "00D8 0F", "atari2600"},
    };
    
    // === DOS (dosbox_pure) ===
    
    // DOOM (DOS)
    m_cheats["doom"] = {
        {"God Mode", "Invincibility", "iddqd", "dos"},
        {"All Weapons", "Full arsenal", "idkfa", "dos"},
        {"No Clipping", "Walk through walls", "idspispopd", "dos"},
    };
    
    // Golden Sun (GBA)
    m_cheats["golden sun"] = {
        {"Max Gold", "999999 coins", "82024BC8 423F\n82024BCA 000F", "gba"},
        {"All Djinn", "All Djinn unlocked", "32003A41 0001", "gba"},
        {"Max Level", "All party members level 99", "32003232 0063", "gba"},
    };
    
    // Final Fantasy VI Advance (GBA)
    m_cheats["final fantasy vi"] = {
        {"Max GP", "999999 GP", "82024BB0 423F\n82024BB2 000F", "gba"},
        {"All Magicite", "All espers", "32003B00 00FF", "gba"},
        {"Max Level", "Party level 99", "32003410 0063", "gba"},
    };
    
    // Metroid Fusion (GBA)
    m_cheats["metroid fusion"] = {
        {"Infinite Health", "Energy never depletes", "82029698 03E7", "gba"},
        {"Infinite Missiles", "999 missiles", "82029740 03E7", "gba"},
        {"All Items", "Full equipment", "82029710 FFFF", "gba"},
    };
    
    // Advance Wars (GBA)
    m_cheats["advance wars"] = {
        {"Max Funds", "Unlimited money", "82032C20 423F", "gba"},
        {"Instant Win", "Win current mission", "32003408 0001", "gba"},
    };
    
    // Fire Emblem (GBA)
    m_cheats["fire emblem"] = {
        {"Max Gold", "999999 gold", "8202BCB8 423F", "gba"},
        {"All Items", "Full convoy", "32003F00 00FF", "gba"},
        {"Max Stats", "All units max stats", "32004100 63", "gba"},
    };
    
    // === GameCube Games ===
    
    // Super Smash Bros. Melee (GameCube)
    m_cheats["super smash bros melee"] = {
        {"Unlock All Characters", "All fighters unlocked", "04453FC8 FFFFFFFF", "gamecube"},
        {"Unlock All Stages", "All stages available", "04453FCC FFFFFFFF", "gamecube"},
        {"Infinite Health P1", "Player 1 never loses stocks", "04453D10 00000000", "gamecube"},
        {"Always Have Hammer", "Permanent hammer item", "04453D20 00000001", "gamecube"},
    };
    
    // The Legend of Zelda: Wind Waker (GameCube)
    m_cheats["wind waker"] = {
        {"Infinite Health", "Hearts never decrease", "04401E70 43480000", "gamecube"},
        {"Max Rupees", "999 rupees", "00401E74 000003E7", "gamecube"},
        {"Infinite Arrows", "999 arrows", "00401E78 00000063", "gamecube"},
        {"Infinite Bombs", "999 bombs", "00401E7C 00000063", "gamecube"},
        {"All Items", "Full inventory", "04401E80 FFFFFFFF", "gamecube"},
    };
    
    m_cheats["zelda wind waker"] = m_cheats["wind waker"];
    
    // The Legend of Zelda: Twilight Princess (GameCube)
    m_cheats["twilight princess"] = {
        {"Infinite Health", "Hearts never decrease", "04400000 43700000", "gamecube"},
        {"Max Rupees", "9999 rupees", "04400100 0000270F", "gamecube"},
        {"Infinite Arrows", "Never run out", "04400200 00000063", "gamecube"},
        {"All Heart Pieces", "Max heart containers", "04400300 00000014", "gamecube"},
    };
    
    m_cheats["zelda twilight princess"] = m_cheats["twilight princess"];
    
    // Super Mario Sunshine (GameCube)
    m_cheats["super mario sunshine"] = {
        {"Infinite Lives", "99 lives", "04400F00 00000063", "gamecube"},
        {"Infinite Health", "Never lose health", "04400F04 00000008", "gamecube"},
        {"Max Coins", "999 coins", "04400F08 000003E7", "gamecube"},
        {"Infinite Water", "FLUDD never runs out", "04400F10 42C80000", "gamecube"},
        {"Moon Jump", "Hold A to float", "04400F20 3F800000", "gamecube"},
    };
    
    // Metroid Prime (GameCube)
    m_cheats["metroid prime"] = {
        {"Infinite Health", "Energy never depletes", "04401000 43C80000", "gamecube"},
        {"Infinite Missiles", "999 missiles", "04401010 000003E7", "gamecube"},
        {"All Items", "Full equipment", "04401020 FFFFFFFF", "gamecube"},
        {"Infinite Scan Visor", "Never runs out", "04401030 42C80000", "gamecube"},
    };
    
    // Resident Evil 4 (GameCube)
    m_cheats["resident evil 4"] = {
        {"Infinite Health", "Never take damage", "04402000 447A0000", "gamecube"},
        {"Infinite Ammo", "Guns never reload", "04402010 000003E7", "gamecube"},
        {"Max Money", "999999 Pesetas", "04402020 000F423F", "gamecube"},
        {"All Weapons", "Full arsenal", "04402030 FFFFFFFF", "gamecube"},
    };
    
    // Animal Crossing (GameCube)
    m_cheats["animal crossing"] = {
        {"Max Bells", "999999 bells", "04403000 000F423F", "gamecube"},
        {"All Items", "Full catalog", "04403010 FFFFFFFF", "gamecube"},
        {"No Weeds", "Town stays clean", "04403020 00000000", "gamecube"},
    };
    
    // Mario Kart: Double Dash (GameCube)
    m_cheats["mario kart double dash"] = {
        {"Always First Place", "Win every race", "04404000 00000001", "gamecube"},
        {"Infinite Items", "Items never run out", "04404010 00000001", "gamecube"},
        {"Unlock All Characters", "All racers available", "04404020 FFFFFFFF", "gamecube"},
        {"Unlock All Karts", "All vehicles unlocked", "04404030 FFFFFFFF", "gamecube"},
    };
    
    // === Wii Games ===
    
    // Super Smash Bros. Brawl (Wii)
    m_cheats["super smash bros brawl"] = {
        {"Unlock All Characters", "All fighters unlocked", "04500000 FFFFFFFF", "wii"},
        {"Unlock All Stages", "All stages available", "04500010 FFFFFFFF", "wii"},
        {"Infinite Stocks", "Never lose a stock", "04500020 00000000", "wii"},
        {"One Hit KO", "Instant knockouts", "04500030 00000001", "wii"},
    };
    
    // Mario Kart Wii
    m_cheats["mario kart wii"] = {
        {"Always First Place", "Win every race", "04501000 00000001", "wii"},
        {"Infinite Items", "Items never run out", "04501010 00000001", "wii"},
        {"Unlock All Characters", "All racers available", "04501020 FFFFFFFF", "wii"},
        {"Unlock All Vehicles", "All karts/bikes unlocked", "04501030 FFFFFFFF", "wii"},
        {"Max VR Points", "9999 VR online", "04501040 0000270F", "wii"},
    };
    
    // New Super Mario Bros. Wii
    m_cheats["new super mario bros wii"] = {
        {"Infinite Lives", "99 lives", "04502000 00000063", "wii"},
        {"Always Invincible", "Star power always active", "04502010 00000001", "wii"},
        {"Infinite Time", "Timer never runs out", "04502020 000001C2", "wii"},
        {"All Star Coins", "All coins collected", "04502030 FFFFFFFF", "wii"},
    };
    
    // The Legend of Zelda: Skyward Sword (Wii)
    m_cheats["skyward sword"] = {
        {"Infinite Health", "Hearts never decrease", "04503000 43700000", "wii"},
        {"Max Rupees", "9900 rupees", "04503010 000026AC", "wii"},
        {"Infinite Arrows", "Never run out", "04503020 00000063", "wii"},
        {"Infinite Bombs", "Never run out", "04503030 00000063", "wii"},
        {"All Items", "Full inventory", "04503040 FFFFFFFF", "wii"},
    };
    
    m_cheats["zelda skyward sword"] = m_cheats["skyward sword"];
    
    // Super Mario Galaxy (Wii)
    m_cheats["super mario galaxy"] = {
        {"Infinite Health", "3 health always", "04504000 00000003", "wii"},
        {"Max Star Bits", "9999 star bits", "04504010 0000270F", "wii"},
        {"Infinite Lives", "99 lives", "04504020 00000063", "wii"},
        {"Moon Jump", "Hold A to float", "04504030 3F800000", "wii"},
    };
    
    // Super Mario Galaxy 2 (Wii)
    m_cheats["super mario galaxy 2"] = {
        {"Infinite Health", "3 health always", "04505000 00000003", "wii"},
        {"Max Star Bits", "9999 star bits", "04505010 0000270F", "wii"},
        {"Infinite Lives", "99 lives", "04505020 00000063", "wii"},
        {"All Stars", "120 stars unlocked", "04505030 00000078", "wii"},
    };
    
    // Donkey Kong Country Returns (Wii)
    m_cheats["donkey kong country returns"] = {
        {"Infinite Lives", "99 lives", "04506000 00000063", "wii"},
        {"Infinite Health", "Never lose hearts", "04506010 00000002", "wii"},
        {"All Puzzle Pieces", "100% completion", "04506020 FFFFFFFF", "wii"},
        {"Max Banana Coins", "999 coins", "04506030 000003E7", "wii"},
    };
    
    // Kirby's Return to Dream Land (Wii)
    m_cheats["kirby return to dream land"] = {
        {"Infinite Lives", "99 lives", "04507000 00000063", "wii"},
        {"Infinite Health", "Never lose HP", "04507010 00000006", "wii"},
        {"All Copy Abilities", "Every power unlocked", "04507020 FFFFFFFF", "wii"},
        {"Max Energy Spheres", "All spheres collected", "04507030 00000078", "wii"},
    };
    
    // Xenoblade Chronicles (Wii)
    m_cheats["xenoblade chronicles"] = {
        {"Max HP All Characters", "Full health party", "04508000 0001869F", "wii"},
        {"Infinite AP", "Max ability points", "04508010 3B9AC9FF", "wii"},
        {"Max Money", "999999 gold", "04508020 000F423F", "wii"},
        {"All Arts Unlocked", "Every skill available", "04508030 FFFFFFFF", "wii"},
    };
    
    std::cout << "Loaded " << m_cheats.size() << " games into cheat database" << std::endl;
}

std::vector<CheatCode> CheatDatabase::getCheatsForRom(const std::string& romName, const std::string& system) {
    std::string normalized = normalizeRomName(romName);
    
    // Try exact match first
    auto it = m_cheats.find(normalized);
    if (it != m_cheats.end()) {
        // Filter by system if specified
        if (!system.empty()) {
            std::vector<CheatCode> filtered;
            for (const auto& cheat : it->second) {
                if (cheat.system == system) {
                    filtered.push_back(cheat);
                }
            }
            return filtered;
        }
        return it->second;
    }
    
    // Try partial match
    for (const auto& entry : m_cheats) {
        if (normalized.find(entry.first) != std::string::npos ||
            entry.first.find(normalized) != std::string::npos) {
            
            if (!system.empty()) {
                std::vector<CheatCode> filtered;
                for (const auto& cheat : entry.second) {
                    if (cheat.system == system) {
                        filtered.push_back(cheat);
                    }
                }
                return filtered;
            }
            return entry.second;
        }
    }
    
    return {}; // No cheats found
}

std::vector<CheatCode> CheatDatabase::searchCheats(const std::string& query, const std::string& system) {
    std::vector<CheatCode> results;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(),
        [](unsigned char c){ return std::tolower(c); });
    
    for (const auto& gameEntry : m_cheats) {
        for (const auto& cheat : gameEntry.second) {
            // Filter by system if specified
            if (!system.empty() && cheat.system != system) {
                continue;
            }
            
            // Search in cheat name and description
            std::string lowerName = cheat.name;
            std::string lowerDesc = cheat.description;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                [](unsigned char c){ return std::tolower(c); });
            std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(),
                [](unsigned char c){ return std::tolower(c); });
            
            if (lowerName.find(lowerQuery) != std::string::npos ||
                lowerDesc.find(lowerQuery) != std::string::npos) {
                results.push_back(cheat);
            }
        }
    }
    
    return results;
}

bool CheatDatabase::addCustomCheat(const CheatCode& cheat) {
    std::string gameName = normalizeRomName(cheat.name);
    m_cheats[gameName].push_back(cheat);
    return true;
}

bool CheatDatabase::loadDatabase(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    // TODO: Implement loading from external cheat database file
    // Format: [Game Name]
    //         cheat_name=code|description|system
    
    return true;
}

bool CheatDatabase::saveDatabase(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    // TODO: Implement saving custom cheats
    
    return true;
}
