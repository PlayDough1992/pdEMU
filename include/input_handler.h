#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <SDL2/SDL.h>
#include <map>
#include <string>

// Include libretro.h for the device IDs to ensure consistency
#include "libretro.h" 

enum class ControlScheme {
    GBA,        // Game Boy Advance
    SNES,       // Super Nintendo (also for NES)
    N64,        // Nintendo 64
    GENESIS,    // Sega Genesis/Mega Drive
    PSX         // PlayStation
};

class InputHandler {
public:
    InputHandler();
    ~InputHandler();

    // Call this once per frame before the core requests input
    void update(); 
    
    // <--- FIX: This declaration MUST exist for the definition in the .cpp file to match
    void poll();
    
    // Process SDL events (must be called in your main loop)
    void handleEvent(const SDL_Event& event); 
    
    // The main function called by the libretro core (retro_input_state_t)
    int16_t getInputState(unsigned port, unsigned device, unsigned index, unsigned id);
    
    bool shouldQuit() const { return m_quit; }
    void setQuit(bool quit) { m_quit = quit; }
    
    // Set control scheme based on system
    void setControlScheme(ControlScheme scheme);
    void setControlScheme(const std::string& coreName);

private:
    // Keyboard mappings for different systems (SDL_Keycode -> RETRO_DEVICE_ID_JOYPAD_*)
    std::map<SDL_Keycode, unsigned> m_keyMap;
    
    // Gamepad support
    SDL_GameController* m_controller;
    
    // Combined Input state for Player 1 (RetroPad)
    bool m_buttonStates[16]; 
    bool m_quit;
    ControlScheme m_currentScheme;
    
    // Analog stick state (ranges from -32768 to 32767)
    int16_t m_analogLeftX;
    int16_t m_analogLeftY;
    int16_t m_analogRightX;
    int16_t m_analogRightY;
    
    // Analog Triggers (0 to 32767)
    int16_t m_analogL2; 
    int16_t m_analogR2; 
    
    // Pointer/touchscreen state (for DS touchscreen)
    int16_t m_pointerX;
    int16_t m_pointerY;
    bool m_pointerPressed;
    
    void initKeyMappings();
    void initGBAMappings();
    void initSNESMappings();
    void initN64Mappings();
    void initGenesisMappings();
    void initPSXMappings();
    
    // Updates the digital and analog states based on the keyboard
    void updateKeyboard();
    
    // Updates the digital and analog states based on the gamepad
    void updateGamepad();
};

#endif // INPUT_HANDLER_H