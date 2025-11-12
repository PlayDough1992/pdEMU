# Universal RetroArch Frontend

A multi-system emulator frontend that supports all RetroArch/libretro cores. Play games from dozens of classic systems including Game Boy Advance, NES, SNES, Genesis, PlayStation, and many more!

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

## Supported Systems

| System | Extensions | Recommended Core |
|--------|-----------|------------------|
| Game Boy Advance | .gba | mgba_libretro.so |
| Game Boy / GBC | .gb, .gbc | gambatte_libretro.so |
| NES | .nes | fceumm_libretro.so |
| SNES | .smc, .sfc | snes9x_libretro.so |
| Nintendo 64 | .n64, .z64 | mupen64plus_next_libretro.so |
| Nintendo DS | .nds | desmume_libretro.so |
| GameCube | .iso, .gcm, .gcz | dolphin_libretro.so |
| Wii | .iso, .wbfs, .wad | dolphin_libretro.so |
| Genesis / Mega Drive | .md, .gen | genesis_plus_gx_libretro.so |
| Sega Master System | .sms | genesis_plus_gx_libretro.so |
| Game Gear | .gg | genesis_plus_gx_libretro.so |
| PlayStation | .cue, .chd | beetle_psx_hw_libretro.so |
| PSP | .iso, .cso | ppsspp_libretro.so |
| Arcade | .zip | fbneo_libretro.so |
| And many more! | | |

## Prerequisites

Before building, you need to install the following dependencies:

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsdl2-dev
```

## BIOS Files

Some systems require BIOS files to run. See [BIOS_GUIDE.md](BIOS_GUIDE.md) for a complete list.

**Quick Summary:**
- **Not needed:** GBA, GB/GBC, NES, SNES, N64, Genesis, PSP, most arcade
- **Required:** PlayStation (scph5501.bin), PS2, Saturn, Dreamcast, Nintendo DS

Place BIOS files in the `BIOS/` directory.

## Getting Cores

Before building, you need to install the following dependencies:

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsdl2-dev
```

### Fedora
```bash
sudo dnf install gcc-c++ cmake SDL2-devel
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake sdl2
```

## Getting the mGBA Core

You need to obtain the mGBA libretro core file (`mgba_libretro.so`). You have several options:

### Option 1: Download Pre-built Core
Download from the libretro buildbot:
```bash
mkdir -p cores
cd cores
wget https://buildbot.libretro.com/nightly/linux/x86_64/latest/mgba_libretro.so.zip
unzip mgba_libretro.so.zip
cd ..
```

### Option 2: Build from Source
```bash
git clone https://github.com/libretro/mgba.git
cd mgba
mkdir build && cd build
cmake .. -DLIBRETRO=ON -DBUILD_LIBRETRO=ON
make
# The core will be in build/mgba_libretro.so
```

## Building the Frontend

### Using Make (Recommended)
```bash
make
```

The executable will be created at `build/mGBA_Frontend`.

### Using CMake
```bash
mkdir build
cd build
cmake ..
make
cd ..
```

## Usage

```bash
./build/mGBA_Frontend <path_to_mgba_libretro.so> <path_to_rom.gba>
```

### Example
```bash
./build/mGBA_Frontend cores/mgba_libretro.so ROMS/game.gba
```

## Directory Structure

The project uses the following directory layout:
- `cores/` - Libretro core files (*.so)
- `ROMS/` - Game ROM files organized by system
- `BIOS/` - System BIOS files (when required)
- `SAVES/` - Save files and save states
- `LOGO/` - Splash screen logo (pdEMU_LOGO.png)

These directories will be automatically referenced by the emulator.

## Controls

### Keyboard
| Key | GBA Button |
|-----|------------|
| Arrow Keys | D-Pad |
| Z | A Button |
| X | B Button |
| A | L Trigger |
| S | R Trigger |
| Enter | Start |
| Shift | Select |
| ESC | Quit |
| Tab | Fast-Forward |
| F1 | Toggle FPS Counter |
| F2 | Decrease Scale |
| F3 | Increase Scale |
| F4 | Toggle Filter |
| F5 | Save State (coming soon) |
| F9 | Load State (coming soon) |

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

This frontend interfaces with the mGBA libretro core using the libretro API:

1. **Core Loading**: The frontend dynamically loads the mGBA shared library (.so file)
2. **Callbacks**: Registers callbacks for video frames, audio samples, and input polling
3. **Game Loop**: Runs the emulation loop at ~60 FPS (GBA native speed)
4. **Rendering**: Converts core video output to SDL2 textures for display
5. **Audio**: Buffers audio samples and plays them through SDL2's audio system
6. **Input**: Maps keyboard/gamepad input to GBA button states

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
- Make sure the path to the mGBA core file is correct
- Verify the core file has execute permissions: `chmod +x mgba_libretro.so`
- Check if you're using the correct architecture (x86_64 vs ARM)

### "Failed to open game file"
- Verify the ROM file path is correct
- Ensure the ROM file is a valid GBA ROM (.gba extension)

### No audio
- Check if SDL2 audio is properly configured on your system
- Try running: `aplay -l` to list audio devices

### Performance issues
- The emulator targets 60 FPS with VSync enabled
- On slower systems, you may experience frame drops
- Try closing other applications to free up resources

## BIOS File (Optional)

The GBA BIOS file is optional but recommended for better compatibility. If you have a `gba_bios.bin` file, place it in the `BIOS/` directory:

```bash
mkdir -p BIOS
# Copy your gba_bios.bin to BIOS/
cp gba_bios.bin BIOS/
```

The emulator will automatically use it if present. Most games work fine without it using the built-in HLE BIOS in mGBA.

## Extending the Frontend

Want to add more features? Here are some ideas:

- **Save States**: Implement F5/F9 hotkeys for save/load states
- **Config File**: Add support for custom key mappings via config file
- **Cheats**: Interface with the core's cheat system
- **Fast Forward**: Add speed control (2x, 4x, etc.)
- **Screenshots**: Capture and save video frames
- **On-Screen Display**: Show FPS, frame time, etc.
- **GUI Menu**: Add ImGui for settings and ROM selection

## License

This frontend code is provided as-is for educational purposes. The mGBA core itself is licensed under the Mozilla Public License 2.0. Please respect the licenses of all components.

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

Feel free to fork this project and add your own features! Some areas that could use improvement:
- Better error handling
- Configuration file support
- More robust audio buffering
- Additional core support (NES, SNES, etc.)

## Author

Created as a demonstration of how to build a custom RetroArch core frontend.
