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
if [ -f "build/mGBA_Frontend" ]; then
    echo "✓ Executable found: build/mGBA_Frontend"
    echo ""
    echo "Launching pdEMU Frontend..."
    echo ""
    ./build/mGBA_Frontend
else
    echo "Executable not found. Building..."
    echo ""
    make
    echo ""
    if [ -f "build/mGBA_Frontend" ]; then
        echo "✓ Build successful!"
        echo ""
        echo "Launching pdEMU Frontend..."
        echo ""
        ./build/mGBA_Frontend
    else
        echo "✗ Build failed. Please check the errors above."
        exit 1
    fi
fi
