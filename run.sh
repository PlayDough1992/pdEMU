#!/bin/bash#!/bin/bash

# MSYS2 build and run script for pdEMU on Windows

echo "==================================="

echo "==================================="echo "pdEMU Frontend - Build & Run Script"

echo "pdEMU - Build & Run (Windows/MSYS2)"echo "==================================="

echo "==================================="echo ""

echo ""

# Check if build directory exists

# Check if executable existsif [ ! -d "build" ]; then

if [ -f "build/pdEMU.exe" ]; then    echo "Build directory not found. Creating..."

    echo "Executable found: build/pdEMU.exe"    mkdir -p build

    echo ""fi

    echo "Launching pdEMU..."

    echo ""# Check if already built

    ./build/pdEMU.exeif [ -f "build/pdEMU.exe" ]; then

else    echo "✓ Executable found: build/pdEMU.exe"

    echo "Executable not found. Building..."    echo ""

    echo ""    echo "Launching pdEMU Frontend..."

        echo ""

    # Create build directory if needed    ./build/pdEMU.exe

    mkdir -p buildelse

    cd build    echo "Executable not found. Building..."

        echo ""

    # Configure and build    

    cmake -G "MinGW Makefiles" ..    cd build

    mingw32-make    cmake -G "MinGW Makefiles" ..

        mingw32-make

    cd ..    cd ..

        

    echo ""    echo ""

    if [ -f "build/pdEMU.exe" ]; then    if [ -f "build/pdEMU.exe" ]; then

        echo "Build successful!"        echo "✓ Build successful!"

        echo ""        echo ""

        echo "Launching pdEMU..."        echo "Launching pdEMU Frontend..."

        echo ""        echo ""

        ./build/pdEMU.exe        ./build/pdEMU.exe

    else    else

        echo "Build failed. Please check the errors above."        echo "✗ Build failed. Please check the errors above."

        exit 1        exit 1

    fi    fi

fifi

