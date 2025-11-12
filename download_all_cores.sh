#!/bin/bash

# Script to download popular libretro cores from the official buildbot

set -e

CORES_DIR="cores"
BUILDBOT_URL="https://buildbot.libretro.com/nightly/linux/x86_64/latest"

echo "==================================="
echo "Downloading Libretro Cores"
echo "==================================="

mkdir -p "$CORES_DIR"
cd "$CORES_DIR"

# Function to download and extract a core
download_core() {
    local core_name=$1
    echo "Downloading ${core_name}..."
    
    if command -v wget &> /dev/null; then
        wget -q "${BUILDBOT_URL}/${core_name}.so.zip" || echo "Failed to download ${core_name}"
    elif command -v curl &> /dev/null; then
        curl -sL -O "${BUILDBOT_URL}/${core_name}.so.zip" || echo "Failed to download ${core_name}"
    else
        echo "Error: Neither wget nor curl found"
        exit 1
    fi
    
    if [ -f "${core_name}.so.zip" ]; then
        unzip -q -o "${core_name}.so.zip" 2>/dev/null
        rm "${core_name}.so.zip"
        chmod +x "${core_name}.so" 2>/dev/null
        echo "✓ ${core_name}.so"
    fi
}

echo ""
echo "Downloading cores (this may take a few minutes)..."
echo ""

# Nintendo cores
download_core "mgba_libretro"              # Game Boy Advance
download_core "gambatte_libretro"          # Game Boy / Game Boy Color
download_core "fceumm_libretro"            # NES
download_core "snes9x_libretro"            # SNES
download_core "mupen64plus_next_libretro"  # Nintendo 64
download_core "desmume_libretro"           # Nintendo DS
download_core "melonds_libretro"           # Nintendo DS (alternative)
download_core "dolphin_libretro"           # GameCube / Wii

# Sega cores
download_core "genesis_plus_gx_libretro"   # Genesis/Mega Drive/SMS/Game Gear
download_core "picodrive_libretro"         # Genesis/Mega Drive (alternative)
download_core "flycast_libretro"           # Dreamcast
download_core "beetle_saturn_libretro"     # Saturn

# Sony cores
download_core "beetle_psx_hw_libretro"     # PlayStation (hardware)
download_core "beetle_psx_libretro"        # PlayStation (software)
download_core "pcsx_rearmed_libretro"      # PlayStation (ARM optimized)
download_core "ppsspp_libretro"            # PSP

# Arcade
download_core "fbneo_libretro"             # Arcade (FinalBurn Neo)
download_core "mame_libretro"              # Arcade (MAME)

# Other systems
download_core "stella_libretro"            # Atari 2600
download_core "prosystem_libretro"         # Atari 7800
download_core "beetle_pce_fast_libretro"   # PC Engine / TurboGrafx-16
download_core "beetle_ngp_libretro"        # Neo Geo Pocket
download_core "beetle_wswan_libretro"      # WonderSwan
download_core "dosbox_pure_libretro"       # DOS

cd ..

echo ""
echo "==================================="
echo "Core Download Complete!"
echo "==================================="
echo ""
echo "Downloaded cores to: ./$CORES_DIR/"
echo ""
ls -lh "$CORES_DIR"/*.so 2>/dev/null | wc -l | xargs echo "Total cores:"
echo ""
echo "NOTE: BIOS files must be obtained legally from your own hardware."
echo "See BIOS_GUIDE.md for required BIOS files and where to place them."
echo ""
