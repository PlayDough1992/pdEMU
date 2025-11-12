#!/bin/bash

# Script to download the mGBA libretro core

set -e

CORE_DIR="cores"
CORE_FILE="mgba_libretro.so"
DOWNLOAD_URL="https://buildbot.libretro.com/nightly/linux/x86_64/latest/mgba_libretro.so.zip"

echo "==================================="
echo "Downloading mGBA Libretro Core"
echo "==================================="

# Create cores directory
mkdir -p "$CORE_DIR"

# Download the core
echo "Downloading from libretro buildbot..."
cd "$CORE_DIR"

if command -v wget &> /dev/null; then
    wget -O mgba_libretro.so.zip "$DOWNLOAD_URL"
elif command -v curl &> /dev/null; then
    curl -L -o mgba_libretro.so.zip "$DOWNLOAD_URL"
else
    echo "Error: Neither wget nor curl found. Please install one of them."
    exit 1
fi

# Extract
echo "Extracting core..."
if command -v unzip &> /dev/null; then
    unzip -o mgba_libretro.so.zip
    rm mgba_libretro.so.zip
else
    echo "Error: unzip not found. Please install it."
    exit 1
fi

# Make executable
chmod +x "$CORE_FILE"

cd ..

echo ""
echo "==================================="
echo "Download complete!"
echo "==================================="
echo ""
echo "Core location: ./$CORE_DIR/$CORE_FILE"
echo ""
echo "You can now run:"
echo "./build/mGBA_Frontend cores/mgba_libretro.so ROMS/your_game.gba"
