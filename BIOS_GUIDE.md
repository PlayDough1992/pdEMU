# BIOS Files Guide

This document lists the BIOS files needed for various systems. Place all BIOS files in the `BIOS/` directory.

## Required BIOS Files by System

### Sony PlayStation (PSX)
**Required:** Yes
```
BIOS/
├── scph5500.bin  (Japan)
├── scph5501.bin  (USA) - RECOMMENDED
└── scph5502.bin  (Europe)
```
- MD5: 490f666e1afb15b7362b406ed1cea246 (scph5501.bin)
- Only one region BIOS is needed, but scph5501.bin (USA) is most compatible

### Sony PlayStation 2 (PS2)
**Required:** Yes
```
BIOS/
├── SCPH10000.bin  (Japan v5.0)
├── SCPH39001.bin  (USA v7.0) - RECOMMENDED
└── SCPH70012.bin  (USA v12.0)
```
- USA v7.0+ recommended for best compatibility

### Sega Saturn
**Required:** Yes
```
BIOS/
├── saturn_bios.bin  (Any region)
└── mpr-17933.bin    (Alternative name)
```
- MD5: af5828fdff51384f99b3c4926be27762

### Sega Dreamcast
**Required:** Yes
```
BIOS/
├── dc_boot.bin
└── dc_flash.bin
```

### Nintendo DS
**Required:** Yes
```
BIOS/
├── bios7.bin      (ARM7 BIOS)
├── bios9.bin      (ARM9 BIOS)
└── firmware.bin   (Firmware)
```

### Game Boy Advance
**Required:** No (but improves compatibility)
```
BIOS/
└── gba_bios.bin
```
- MD5: a860e8c0b6d573d191e4ec7db1b1e4f6
- mGBA includes a High-Level Emulation (HLE) BIOS, but some games work better with official BIOS

### Game Boy / Game Boy Color
**Required:** No
```
BIOS/
├── gb_bios.bin
└── gbc_bios.bin
```
- Optional, most games work fine without

## Systems That DON'T Need BIOS

The following systems work perfectly without any BIOS files:

- ✅ NES (Nintendo Entertainment System)
- ✅ SNES (Super Nintendo)
- ✅ Nintendo 64
- ✅ Sega Genesis / Mega Drive
- ✅ Sega Master System
- ✅ Sega Game Gear
- ✅ PC Engine / TurboGrafx-16
- ✅ Neo Geo Pocket
- ✅ WonderSwan
- ✅ Atari 2600
- ✅ Atari 7800
- ✅ PSP (PlayStation Portable)
- ✅ Most Arcade games (some need ROM sets)

## How to Obtain BIOS Files

**Important:** You must own the original hardware to legally use BIOS files. BIOS files are copyrighted and cannot be distributed.

1. **Dump from your own console** - The legal way
2. **Purchase used consoles** - Extract BIOS using homebrew tools
3. **Never download from the internet** - This is piracy

## Checking Your BIOS Files

Run this command to verify BIOS file MD5 checksums:
```bash
md5sum BIOS/*.bin
```

Compare with the MD5 hashes listed above for each system.

## Troubleshooting

### "Failed to load game" or black screen
- Check if the system requires a BIOS
- Verify BIOS filename matches exactly (case-sensitive on Linux)
- Check MD5 hash to ensure file isn't corrupt

### Wrong region or language
- Some systems need specific regional BIOS files
- PlayStation: Use scph5501.bin for USA games, scph5502.bin for PAL

### Performance issues
- Some BIOS files (like PS2) are large and slow to load
- This is normal and only happens once at startup

## Quick Reference

| System | BIOS Required | Files Needed |
|--------|--------------|--------------|
| GBA | No (optional) | gba_bios.bin |
| GB/GBC | No | - |
| NES | No | - |
| SNES | No | - |
| N64 | No | - |
| NDS | **Yes** | bios7.bin, bios9.bin, firmware.bin |
| Genesis | No | - |
| Saturn | **Yes** | saturn_bios.bin |
| Dreamcast | **Yes** | dc_boot.bin, dc_flash.bin |
| PSX | **Yes** | scph5501.bin (USA) |
| PS2 | **Yes** | SCPH39001.bin (USA) |
| PSP | No | - |
| Arcade | No* | *Some need specific ROM sets |

## File Structure Example

```
RA_mGBA_CORE_BASED_EMU/
├── BIOS/
│   ├── gba_bios.bin           (optional)
│   ├── scph5501.bin           (PSX USA)
│   ├── SCPH39001.bin          (PS2 USA)
│   ├── saturn_bios.bin        (Saturn)
│   ├── dc_boot.bin            (Dreamcast)
│   ├── dc_flash.bin           (Dreamcast)
│   ├── bios7.bin              (NDS)
│   ├── bios9.bin              (NDS)
│   └── firmware.bin           (NDS)
├── cores/
├── ROMS/
└── SAVES/
```

## Notes

- BIOS files are system-specific, not game-specific
- One BIOS file can run all games for that system
- Keep BIOS files in the `BIOS/` directory (case-sensitive)
- The frontend automatically looks for BIOS files in this directory
- Region-free BIOS files are preferable when available
