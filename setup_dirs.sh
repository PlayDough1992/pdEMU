#!/bin/bash

# Script to create necessary directories

echo "Creating directory structure..."

mkdir -p cores
mkdir -p ROMS
mkdir -p BIOS
mkdir -p SAVES

echo "Directory structure created:"
echo "  cores/  - Place your libretro cores here (mgba_libretro.so)"
echo "  ROMS/   - Place your GBA ROM files here"
echo "  BIOS/   - Place gba_bios.bin here (optional)"
echo "  SAVES/  - Save files and states will be stored here"
echo ""
echo "Done!"
