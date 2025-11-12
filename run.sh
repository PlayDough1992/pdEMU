#!/bin/bash

echo "==================================="
echo "pdEMU Frontend - Build & Run Script"
echo "==================================="
echo ""

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Creating..."
    mkdir -p build
fi

# Check if already built
if [ -f "build/pdEMU.exe" ]; then
    echo "✓ Executable found: build/pdEMU.exe"
    echo ""
    echo "Launching pdEMU Frontend..."
    echo ""
    ./build/pdEMU.exe
else
    echo "Executable not found. Building..."
    echo ""
    
    cd build
    cmake -G "MinGW Makefiles" ..
    mingw32-make
    cd ..
    
    echo ""
    if [ -f "build/pdEMU.exe" ]; then
        echo "✓ Build successful!"
        echo ""
        echo "Launching pdEMU Frontend..."
        echo ""
        ./build/pdEMU.exe
    else
        echo "✗ Build failed. Please check the errors above."
        exit 1
    fi
fi
