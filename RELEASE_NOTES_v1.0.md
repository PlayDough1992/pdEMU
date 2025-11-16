# pdEMU v1.0 Beta - Windows Release

**Release Date:** November 15, 2025  
**Platform:** Windows x64

---

## 🎮 First Official Windows Release!

This is the first beta release of pdEMU for Windows - a universal multi-system emulator powered by Libretro cores with limited controller support (PS5 controller) and an optimized user experience.

---

## ✨ Key Features

### Multi-System Emulation - Tested and working
- **Nintendo Entertainment System** 
- **Super Nintendo Entertainment System** 
- **Nintendo Gameboy Advance**
- **Nintendo 64**
- **PlayStation 1**
- The list of tested cores from libretro is small for now, you may try to add more cores but they are untested at this time and may cause freezing/crashing and controller support may or may not work.

### Controller Support
- **Full UI Navigation** - Navigate everything with your controller (no mouse/keyboard needed)
  - ROM browser with D-pad/analog stick
  - On-screen keyboard for search
  - Settings menu navigation
  - Button selection with visual highlights
- **Controller Profile System** - JSON-based profile management with hotplug support
- **Exit Combo** - Press Start+Select during gameplay to return to menu
- **Tested Controllers** - DualSense (PS5)

### User Interface
- **Fullscreen Borderless Splash Screen** - Clean startup experience
- **ImGui-based Interface** - Responsive and lightweight
- **Visual Feedback** - Highlighted buttons and navigation indicators
- **Settings Controls** - Adjust audio volume, video settings, and more via controller

### Performance Optimizations
- **PS1 Audio** - Crystal clear 64-sample buffer with pass-through processing
- **DK64 Flickering Fix** - Frame Pacing/deduplification system prevents the notorius screen flickering issue that Donkey Kong 64 is known to cause.
- **Hardware Rendering** - OpenGL support for N64
- **4:3 Aspect Ratio** - Proper aspect ratio-retaining display scaling

---

## 🐛 Known Issues

- Icon cache may require Explorer restart to display properly
- Some cores may require BIOS files 
- Controller hotplug during gameplay requires menu restart

---

## 📦 Installation

1. Extract the release archive to your desired location
2. Place ROM files in the `roms/` directory (do not create sub-directories, pdEMU organizes them automatically. If it is incorrect, you can use the 'Change System' button to correct it.)
3. Add any required BIOS files to the `BIOS/` directory
4. Run `pdEMU.exe`

---

## 🎯 System Requirements

- **OS:** Windows 10/11 (x64)
- **Graphics:** OpenGL 3.3+ compatible GPU
- **Controller:** Any DirectInput/XInput compatible controller (for best case scenarios, use DualSense (PS5) Wired)
- **Storage:** ~500MB for emulator + space for ROMs

---

## 🔧 Configuration

### Controller Profiles
- Profiles are stored in `controller_profiles/`
- Auto-detected on first launch

---

## 🙏 Credits

- **Libretro** - Core emulation framework
- **ImGui** - User interface library
- **SDL2** - Input, audio, and windowing
- **nlohmann/json** - Configuration management

---

## 📝 Changelog

### v1.0 Beta (November 15, 2025)
- Initial Windows release
- Full controller profile system with JSON persistence
- Complete UI navigation via controller (must use mouse to change system)
- Start+Select exit combo (Exit the current emulation and return to the ROM Manager)
- Audio optimization (64-sample buffer)
- DK64 flickering fix
- Application icon integration
- Fullscreen borderless splash screen
- Settings menu controller navigation
- On-screen keyboard with controller support

---

## 🐞 Bug Reports & Feedback

Please report issues on the GitHub repository:
https://github.com/PlayDough1992/pdEMU

---

**Enjoy gaming! 🎮**
