#!/bin/bash

# Build script for mGBA Frontend

set -e

echo "==================================="
echo "Building mGBA Frontend"
echo "==================================="

# Check for required dependencies
echo "Checking dependencies..."

if ! command -v g++ &> /dev/null; then
    echo "Error: g++ not found. Please install build-essential or equivalent."
    exit 1
fi

if ! pkg-config --exists sdl2; then
    echo "Error: SDL2 not found. Please install libsdl2-dev or equivalent."
    exit 1
fi

echo "All dependencies found!"
echo ""

# Build using Make
echo "Building with Make..."
make clean
make

if [ $? -eq 0 ]; then
    echo ""
    echo "==================================="
    echo "Build successful!"
    echo "==================================="
    echo ""
    echo "Executable: ./build/mGBA_Frontend"
    echo ""
    echo "Usage: ./build/mGBA_Frontend <core_path> <rom_path>"
    echo "Example: ./build/mGBA_Frontend cores/mgba_libretro.so ROMS/game.gba"
    echo ""
    echo "Directory Structure:"
    echo "  cores/  - Libretro cores"
    echo "  ROMS/   - Game ROM files"
    echo "  BIOS/   - System BIOS files (optional for GBA)"
    echo "  SAVES/  - Save files and states (auto-created)"
    echo ""
else
    echo "Build failed!"
    exit 1
fi
