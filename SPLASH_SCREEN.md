# Splash Screen Feature

The pdEMU frontend displays a custom branded splash screen when launching any game.

## How It Works

When you select a ROM from the browser and press "Launch", the following happens:

1. **Splash Window Creation** - A new window opens displaying your logo
2. **Logo Display** - The `LOGO/pdEMU_LOGO.png` image is centered on screen
3. **Duration** - The splash shows for 2 seconds (2000ms) by default
4. **Skip Option** - Press ESC to skip the splash screen immediately
5. **Emulator Launch** - After the splash, the game starts in the emulator window

## Customization

### Change the Logo

Replace `LOGO/pdEMU_LOGO.png` with your own PNG image. The logo will be displayed at its native resolution, centered on screen.

### Adjust Duration

Edit `src/main.cpp` line ~234:
```cpp
splash.show(2000, "Loading...");  // Change 2000 to your preferred duration in milliseconds
```

Examples:
- `splash.show(1000)` - 1 second
- `splash.show(3000)` - 3 seconds
- `splash.show(5000)` - 5 seconds

### Change Window Size

Edit `src/main.cpp` around line ~207:
```cpp
SDL_Window* splashWindow = SDL_CreateWindow(
    "pdEMU",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    1024, 768,  // Change these values for different window size
    SDL_WINDOW_SHOWN
);
```

## Technical Details

The splash screen system uses:
- **SDL2** for window management
- **SDL2_image** for PNG loading
- **SplashScreen class** (`include/splash_screen.h`) for rendering logic

### Key Features:
- Non-blocking event handling (keeps window responsive)
- ESC key to skip
- Black background to match logo
- Automatic centering of logo image
- Clean shutdown before emulator launch

## Future Enhancements

Potential improvements you could add:
1. **Text Rendering** - Add status text using SDL_ttf (e.g., "Loading...", game name)
2. **Fade Effects** - Smooth fade-in/fade-out transitions
3. **Progress Bar** - Show loading progress for larger games
4. **Animated Logo** - Support for animated sprites or GIFs
5. **Per-System Logos** - Different splash screens for different gaming systems

## Implementation Notes

The splash screen is shown **after** ROM selection but **before** core loading. This ensures:
- Fast ROM browser startup (no delay)
- Professional branding during the loading phase
- Universal compatibility (works with all cores)
- No interference with emulator rendering

The splash uses a separate SDL window that is destroyed before the emulator window is created, ensuring clean resource management and no conflicts.
