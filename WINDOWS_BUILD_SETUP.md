# Windows Build Setup Guide

## Prerequisites

You need MSYS2 installed on Windows to build pdEMU.

### Step 1: Install SDL2 and Build Tools

Open **MSYS2 MinGW 64-bit** terminal (not PowerShell) and run:

```bash
# Update package database
pacman -Syu

# Install SDL2
pacman -S mingw-w64-x86_64-SDL2

# Install build tools
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-gcc
```

### Step 2: Build pdEMU

Still in the MSYS2 MinGW 64-bit terminal:

```bash
# Navigate to the project directory
cd /d/Projects_other/pdEMU_Windows

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G "MinGW Makefiles" ..

# Build
mingw32-make

# Or simply
make
```

### Step 3: Get Libretro Cores (DLL files)

You'll need to download Windows versions of libretro cores (.dll files) and place them in the `cores/` directory.

You can download cores from the libretro buildbot:
https://buildbot.libretro.com/nightly/windows/x86_64/latest/

Example cores to download:
- mgba_libretro.dll (Game Boy Advance)
- snes9x_libretro.dll (SNES)
- genesis_plus_gx_libretro.dll (Genesis/Mega Drive)
- beetle_psx_hw_libretro.dll (PlayStation)

### Directory Structure

```
pdEMU_Windows/
├── build/          # Build output
├── cores/          # Libretro .dll cores go here
├── ROMS/           # Your ROM files
├── BIOS/           # BIOS files (for systems that need them)
└── SAVES/          # Save files will be created here
```

### Alternative: Using vcpkg

If you prefer using Visual Studio and vcpkg:

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install SDL2
.\vcpkg install sdl2:x64-windows

# Then use Visual Studio to build the project
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Troubleshooting

**CMake can't find SDL2:**
- Make sure you're running from MSYS2 MinGW 64-bit terminal, not regular PowerShell
- Ensure SDL2 is installed: `pacman -Q mingw-w64-x86_64-SDL2`

**Missing make command:**
- Install it: `pacman -S mingw-w64-x86_64-make`
- In MSYS2, you can use either `make` or `mingw32-make`

**OpenGL errors:**
- OpenGL should be available by default on Windows
- Make sure your graphics drivers are up to date
