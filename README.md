# pdEMU - Universal Libretro Frontend (Windows)

A multi-system emulator frontend for Windows that supports all RetroArch/libretro cores. Play games from dozens of classic systems including Game Boy Advance, NES, SNES, Genesis, PlayStation, and many more!

**Native Windows build with full OpenGL support!**

## Features

- 🎮 **Multi-System Support** - Supports 20+ gaming systems
- 📁 **ROM Browser** - Beautiful GUI with system categories
- 🔄 **Auto-Core Detection** - Automatically selects the right core for your ROMs
- 🖥️ **SDL2 Rendering** - Hardware-accelerated video with scaling options
- 🔊 **Audio Support** - Low-latency audio playback
- ⌨️ **Input Mapping** - Keyboard and gamepad support
- 💾 **Config System** - Persistent settings and preferences
- 🎨 **Internal Resolution Scaling** - Render at higher resolutions for cleaner graphics
- ⚡ **Performance** - Fast-forward, FPS counter, and optimization options
- 🎬 **Splash Screen** - Custom branded logo display on launch
- 🪟 **Windows Support** - Native Windows build with full OpenGL support

## Supported Systems

| System | Extensions | Recommended Core |
|--------|-----------|------------------|
| Game Boy Advance | .gba | mgba_libretro.dll |
| Game Boy / GBC | .gb, .gbc | gambatte_libretro.dll |
| NES | .nes | fceumm_libretro.dll |
| SNES | .smc, .sfc | snes9x_libretro.dll |
| Nintendo 64 | .n64, .z64 | mupen64plus_next_libretro.dll |
| Nintendo DS | .nds | desmume_libretro.dll |
| GameCube | .iso, .gcm, .gcz | dolphin_libretro.dll |
| Wii | .iso, .wbfs, .wad | dolphin_libretro.dll |
| Genesis / Mega Drive | .md, .gen | genesis_plus_gx_libretro.dll |
| Sega Master System | .sms | genesis_plus_gx_libretro.dll |
| Game Gear | .gg | genesis_plus_gx_libretro.dll |
| PlayStation | .cue, .chd | beetle_psx_hw_libretro.dll |
| PSP | .iso, .cso | ppsspp_libretro.dll |
| Arcade | .zip | fbneo_libretro.dll |
| And many more! | | |

## Prerequisites

Before building, you need to install the following dependencies:

### Windows (MSYS2)

See [WINDOWS_BUILD_SETUP.md](WINDOWS_BUILD_SETUP.md) for detailed Windows build instructions.

**Quick start:**
1. Install MSYS2 from https://www.msys2.org/
2. Open MSYS2 MinGW 64-bit terminal
3. Run: `pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-gcc`

## BIOS Files

Some systems require BIOS files to run. See [BIOS_GUIDE.md](BIOS_GUIDE.md) for a complete list.

**Quick Summary:**
- **Not needed:** GBA, GB/GBC, NES, SNES, N64, Genesis, PSP, most arcade
- **Required:** PlayStation (scph5501.bin), PS2, Saturn, Dreamcast, Nintendo DS

Place BIOS files in the `BIOS/` directory.

## Getting Cores

Download Windows cores (.dll files) using the PowerShell script:

```powershell
.\download_cores_windows.ps1
```

This will automatically download popular cores from the libretro buildbot.

Or manually download cores from: https://buildbot.libretro.com/nightly/windows/x86_64/latest/

Place downloaded `.dll` files in the `cores/` directory.

## Building the Frontend

### Using MSYS2 MinGW 64-bit Terminal

**Option 1: Quick build with run script**
```bash
./run.sh
```
This will build and run pdEMU automatically.

**Option 2: Manual build**
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

The executable will be created at `build/pdEMU.exe`.

## Usage

### From PowerShell
```powershell
.\run.ps1
```

### From MSYS2 Terminal
```bash
./build/pdEMU.exe
```

### Direct execution
```powershell
.\build\pdEMU.exe
```

The emulator will launch with a ROM browser GUI. Select a ROM to play!

## Directory Structure

The project uses the following directory layout:
- `cores/` - Libretro core files (*.dll)
- `ROMS/` - Game ROM files organized by system
- `BIOS/` - System BIOS files (when required)
- `SAVES/` - Save files and save states
- `LOGO/` - Splash screen logo (pdEMU_LOGO.png)
- `LOGO/` - Splash screen logo (pdEMU_LOGO.png)

These directories will be automatically referenced by the emulator.

## Controls

### In-Game Keyboard (varies by system)
| Key | Common Mapping |
|-----|----------------|
| Arrow Keys | D-Pad |
| Z | A/Confirm |
| X | B/Cancel |
| A | Y/Action |
| S | X/Action |
| Q | L Trigger |
| W | R Trigger |
| Enter | Start |
| Backspace | Select |
| ESC | Menu |
| Tab | Fast-Forward |
| F1 | Toggle FPS Counter |
| F2 | Decrease Scale |
| F3 | Increase Scale |
| F4 | Toggle Filter |

### Gamepad
Most standard USB gamepads are supported via SDL2's GameController API. The mapping follows the standard layout:
- Face buttons (A/B/X/Y)
- D-Pad
- Shoulder buttons (L/R)
- Start/Select

## Project Structure

```
.
├── include/              # Header files
│   ├── libretro.h       # Libretro API definitions
│   ├── libretro_core.h  # Core wrapper interface
│   ├── video_renderer.h # Video rendering
│   ├── audio_renderer.h # Audio playback
│   └── input_handler.h  # Input management
├── src/                 # Source files
│   ├── main.cpp         # Main application
│   ├── libretro_core.cpp
│   ├── video_renderer.cpp
│   ├── audio_renderer.cpp
│   └── input_handler.cpp
├── cores/               # Libretro cores (mgba_libretro.so)
├── ROMS/                # Game ROM files
├── BIOS/                # System BIOS files
├── SAVES/               # Save files and states
├── CMakeLists.txt       # CMake build configuration
├── Makefile            # Simple Makefile
└── README.md           # This file
```

## How It Works

This frontend interfaces with libretro cores using the libretro API:

1. **Core Loading**: The frontend dynamically loads libretro cores (.so files)
2. **Callbacks**: Registers callbacks for video frames, audio samples, and input polling
3. **Game Loop**: Runs the emulation loop at the target FPS for each system
4. **Rendering**: Converts core video output to SDL2/OpenGL textures for display
5. **Audio**: Buffers audio samples and plays them through SDL2's audio system
6. **Input**: Maps keyboard/gamepad input to system-specific controls

## Libretro API

This project demonstrates how to:
- Load and initialize a libretro core
- Implement environment callbacks
- Handle video frame rendering
- Process audio samples
- Map input devices
- Manage save states and memory

## Troubleshooting

### "Failed to load core"
- Make sure cores are in the `cores/` directory
- Run `./download_all_cores.sh` to get recommended cores
- Verify core files have execute permissions: `chmod +x cores/*.so`
- Check if you're using the correct architecture (x86_64 vs ARM)

### "Failed to open game file"
- Verify the ROM file is supported by the selected system
- Ensure the ROM file has the correct extension
- Some systems may require BIOS files (see BIOS_GUIDE.md)

### No audio
- Check if SDL2 audio is properly configured on your system
- Try running: `aplay -l` to list audio devices

### Performance issues
- The emulator targets 60 FPS with VSync enabled
- On slower systems, you may experience frame drops
- Try closing other applications to free up resources

## BIOS Files

Some systems require BIOS files to run. See [BIOS_GUIDE.md](BIOS_GUIDE.md) for a complete list and instructions.

Place BIOS files in the `BIOS/` directory:

```bash
mkdir -p BIOS
# Copy your BIOS files to BIOS/
cp scph5501.bin BIOS/  # PlayStation example
```

## Extending pdEMU

Want to add more features? Here are some ideas:

- **More Cores**: Add support for additional libretro cores
- **Enhanced GUI**: Improve the ROM browser with cover art, metadata
- **Netplay**: Implement online multiplayer via libretro netplay API
- **Shaders**: Add shader support for advanced visual effects
- **Recording**: Video/audio recording capabilities
- **Achievements**: RetroAchievements integration

## License

This project is open source. The libretro cores maintain their respective licenses (most are MPL 2.0 or GPL). Please respect the licenses of all components.

## Author

Created by PlayDough1992 as a universal frontend for libretro cores.

## Resources

- [Libretro API Documentation](https://docs.libretro.com/)
- [SDL2 Documentation](https://wiki.libsdl.org/)
- [RetroArch Source Code](https://github.com/libretro/RetroArch)

### Project Documentation

- [BIOS_GUIDE.md](BIOS_GUIDE.md) - Complete guide to BIOS requirements
- [BIOS_LEGAL.md](BIOS_LEGAL.md) - Legal information about BIOS distribution
- [SPLASH_SCREEN.md](SPLASH_SCREEN.md) - How to customize the splash screen
- [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) - Technical implementation details

## Contributing

Contributions are welcome! Feel free to fork this project and submit pull requests. Some areas that could use improvement:
- Better error handling and logging
- More comprehensive input configuration
- Additional renderer backends (Vulkan, etc.)
- Performance optimizations
- Cross-platform compatibility (Windows, macOS)

## Author

Created by PlayDough1992 as a universal frontend for libretro cores.

**Repository:** https://github.com/PlayDough1992/pdEMU
