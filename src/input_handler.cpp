#include "input_handler.h"
#include "libretro.h" 
#include <iostream>
#include <algorithm>

// Define a common analog deadzone to prevent stick drift
#define ANALOG_DEADZONE (32767 / 10)

/**
 * @brief Processes an SDL analog axis value, applying a deadzone.
 * @param axisValue The raw SDL axis value (-32768 to 32767).
 * @return The processed axis value, or 0 if within the deadzone.
 */
int16_t processAnalogAxis(int16_t axisValue) {
    if (axisValue > ANALOG_DEADZONE || axisValue < -ANALOG_DEADZONE) {
        return axisValue;
    }
    return 0;
}

InputHandler::InputHandler()
    : m_controller(nullptr)
    , m_quit(false)
    , m_currentScheme(ControlScheme::SNES)
    , m_analogLeftX(0)
    , m_analogLeftY(0)
    , m_analogRightX(0)
    , m_analogRightY(0)
    , m_analogL2(0) 
    , m_analogR2(0)
    , m_pointerX(0)
    , m_pointerY(0)
    , m_pointerPressed(false)
{
    // Initialize all button states to false
    for (int i = 0; i < 16; ++i) {
        m_buttonStates[i] = false;
    }
    
    initKeyMappings();

    // Try to open the first available game controller
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            m_controller = SDL_GameControllerOpen(i);
            if (m_controller) {
                std::cout << "Game controller connected: " 
                         << SDL_GameControllerName(m_controller) << std::endl;
                break;
            }
        }
    }
}

InputHandler::~InputHandler() {
    if (m_controller) {
        SDL_GameControllerClose(m_controller);
        m_controller = nullptr;
    }
}

void InputHandler::setControlScheme(ControlScheme scheme) {
    m_currentScheme = scheme;
    initKeyMappings();
    std::cout << "Control scheme changed to: ";
    switch (scheme) {
        case ControlScheme::GBA: std::cout << "GBA"; break;
        case ControlScheme::SNES: std::cout << "SNES"; break;
        case ControlScheme::N64: std::cout << "N64"; break;
        case ControlScheme::GENESIS: std::cout << "Genesis"; break;
        case ControlScheme::PSX: std::cout << "PlayStation"; break;
    }
    std::cout << std::endl;
}

void InputHandler::setControlScheme(const std::string& coreName) {
    // Auto-detect control scheme from core name (uses substrings for flexibility)
    if (coreName.find("mgba") != std::string::npos || 
        coreName.find("vba") != std::string::npos ||
        coreName.find("gba") != std::string::npos) {
        setControlScheme(ControlScheme::GBA);
    }
    else if (coreName.find("mupen64") != std::string::npos || 
             coreName.find("parallel") != std::string::npos) {
        setControlScheme(ControlScheme::N64);
    }
    else if (coreName.find("genesis") != std::string::npos || 
             coreName.find("picodrive") != std::string::npos) {
        setControlScheme(ControlScheme::GENESIS);
    }
    else if (coreName.find("beetle") != std::string::npos || 
             coreName.find("pcsx") != std::string::npos) {
        setControlScheme(ControlScheme::PSX);
    }
    else {
        // Default to SNES for most 8-bit/16-bit cores
        setControlScheme(ControlScheme::SNES);
    }
}

void InputHandler::initKeyMappings() {
    m_keyMap.clear();
    
    switch (m_currentScheme) {
        case ControlScheme::GBA:
            initGBAMappings();
            break;
        case ControlScheme::SNES:
            initSNESMappings();
            break;
        case ControlScheme::N64:
            initN64Mappings();
            break;
        case ControlScheme::GENESIS:
            initGenesisMappings();
            break;
        case ControlScheme::PSX:
            initPSXMappings();
            break;
    }
}

void InputHandler::initGBAMappings() {
    // GBA: A, B, L, R, Start, Select, D-Pad
    m_keyMap[SDLK_z] = RETRO_DEVICE_ID_JOYPAD_A;
    m_keyMap[SDLK_x] = RETRO_DEVICE_ID_JOYPAD_B;
    m_keyMap[SDLK_a] = RETRO_DEVICE_ID_JOYPAD_L;
    m_keyMap[SDLK_s] = RETRO_DEVICE_ID_JOYPAD_R;
    m_keyMap[SDLK_RETURN] = RETRO_DEVICE_ID_JOYPAD_START;
    m_keyMap[SDLK_BACKSPACE] = RETRO_DEVICE_ID_JOYPAD_SELECT;
    m_keyMap[SDLK_UP] = RETRO_DEVICE_ID_JOYPAD_UP;
    m_keyMap[SDLK_DOWN] = RETRO_DEVICE_ID_JOYPAD_DOWN;
    m_keyMap[SDLK_LEFT] = RETRO_DEVICE_ID_JOYPAD_LEFT;
    m_keyMap[SDLK_RIGHT] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
}

void InputHandler::initSNESMappings() {
    // SNES: A, B, X, Y, L, R, Start, Select, D-Pad
    m_keyMap[SDLK_x] = RETRO_DEVICE_ID_JOYPAD_A;      // A (right)
    m_keyMap[SDLK_z] = RETRO_DEVICE_ID_JOYPAD_B;      // B (bottom)
    m_keyMap[SDLK_s] = RETRO_DEVICE_ID_JOYPAD_X;      // X (top)
    m_keyMap[SDLK_a] = RETRO_DEVICE_ID_JOYPAD_Y;      // Y (left)
    m_keyMap[SDLK_q] = RETRO_DEVICE_ID_JOYPAD_L;      // L shoulder
    m_keyMap[SDLK_w] = RETRO_DEVICE_ID_JOYPAD_R;      // R shoulder
    m_keyMap[SDLK_RETURN] = RETRO_DEVICE_ID_JOYPAD_START;
    m_keyMap[SDLK_BACKSPACE] = RETRO_DEVICE_ID_JOYPAD_SELECT;
    m_keyMap[SDLK_UP] = RETRO_DEVICE_ID_JOYPAD_UP;
    m_keyMap[SDLK_DOWN] = RETRO_DEVICE_ID_JOYPAD_DOWN;
    m_keyMap[SDLK_LEFT] = RETRO_DEVICE_ID_JOYPAD_LEFT;
    m_keyMap[SDLK_RIGHT] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
}

void InputHandler::initN64Mappings() {
    // N64: A, B, C-Buttons, L, R, Z, Start, Analog stick, D-Pad
    
    // Face buttons: X for B, C for A
    m_keyMap[SDLK_x] = RETRO_DEVICE_ID_JOYPAD_B;      // N64 B button
    m_keyMap[SDLK_c] = RETRO_DEVICE_ID_JOYPAD_A;      // N64 A button
    m_keyMap[SDLK_RETURN] = RETRO_DEVICE_ID_JOYPAD_START;
    
    // Shoulder buttons and Z trigger
    m_keyMap[SDLK_a] = RETRO_DEVICE_ID_JOYPAD_L;      // L shoulder
    m_keyMap[SDLK_d] = RETRO_DEVICE_ID_JOYPAD_R;      // R shoulder
    m_keyMap[SDLK_z] = RETRO_DEVICE_ID_JOYPAD_L2;     // Z trigger (mapped to L2)
    
    // D-Pad (IJKL keys)
    m_keyMap[SDLK_j] = RETRO_DEVICE_ID_JOYPAD_LEFT;        // D-pad left
    m_keyMap[SDLK_l] = RETRO_DEVICE_ID_JOYPAD_RIGHT;       // D-pad right
    m_keyMap[SDLK_i] = RETRO_DEVICE_ID_JOYPAD_UP;          // D-pad up
    m_keyMap[SDLK_k] = RETRO_DEVICE_ID_JOYPAD_DOWN;        // D-pad down
    
    // C-Buttons (E, R, F, G mapped to digital buttons X/Y/L3/R3)
    m_keyMap[SDLK_r] = RETRO_DEVICE_ID_JOYPAD_X;           // C-up
    m_keyMap[SDLK_f] = RETRO_DEVICE_ID_JOYPAD_Y;           // C-down
    m_keyMap[SDLK_e] = RETRO_DEVICE_ID_JOYPAD_L3;          // C-left
    m_keyMap[SDLK_g] = RETRO_DEVICE_ID_JOYPAD_R3;          // C-right
}

void InputHandler::initGenesisMappings() {
    // Genesis/Mega Drive: A, B, C, X, Y, Z (6-button), Start
    m_keyMap[SDLK_z] = RETRO_DEVICE_ID_JOYPAD_B;      // A button
    m_keyMap[SDLK_x] = RETRO_DEVICE_ID_JOYPAD_A;      // B button
    m_keyMap[SDLK_c] = RETRO_DEVICE_ID_JOYPAD_R;      // C button
    m_keyMap[SDLK_a] = RETRO_DEVICE_ID_JOYPAD_Y;      // X button
    m_keyMap[SDLK_s] = RETRO_DEVICE_ID_JOYPAD_X;      // Y button
    m_keyMap[SDLK_d] = RETRO_DEVICE_ID_JOYPAD_L;      // Z button
    m_keyMap[SDLK_RETURN] = RETRO_DEVICE_ID_JOYPAD_START;
    m_keyMap[SDLK_UP] = RETRO_DEVICE_ID_JOYPAD_UP;
    m_keyMap[SDLK_DOWN] = RETRO_DEVICE_ID_JOYPAD_DOWN;
    m_keyMap[SDLK_LEFT] = RETRO_DEVICE_ID_JOYPAD_LEFT;
    m_keyMap[SDLK_RIGHT] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
}

void InputHandler::initPSXMappings() {
    // PlayStation: ✕, ○, □, △, L1, L2, R1, R2, Start, Select, L3, R3
    m_keyMap[SDLK_z] = RETRO_DEVICE_ID_JOYPAD_B;      // ✕ (Cross)
    m_keyMap[SDLK_x] = RETRO_DEVICE_ID_JOYPAD_A;      // ○ (Circle)
    m_keyMap[SDLK_a] = RETRO_DEVICE_ID_JOYPAD_Y;      // □ (Square)
    m_keyMap[SDLK_s] = RETRO_DEVICE_ID_JOYPAD_X;      // △ (Triangle)
    m_keyMap[SDLK_q] = RETRO_DEVICE_ID_JOYPAD_L;      // L1
    m_keyMap[SDLK_w] = RETRO_DEVICE_ID_JOYPAD_R;      // R1
    m_keyMap[SDLK_1] = RETRO_DEVICE_ID_JOYPAD_L2;     // L2
    m_keyMap[SDLK_2] = RETRO_DEVICE_ID_JOYPAD_R2;     // R2
    m_keyMap[SDLK_3] = RETRO_DEVICE_ID_JOYPAD_L3;     // L3 (stick click)
    m_keyMap[SDLK_4] = RETRO_DEVICE_ID_JOYPAD_R3;     // R3 (stick click)
    m_keyMap[SDLK_RETURN] = RETRO_DEVICE_ID_JOYPAD_START;
    m_keyMap[SDLK_BACKSPACE] = RETRO_DEVICE_ID_JOYPAD_SELECT;
    m_keyMap[SDLK_UP] = RETRO_DEVICE_ID_JOYPAD_UP;
    m_keyMap[SDLK_DOWN] = RETRO_DEVICE_ID_JOYPAD_DOWN;
    m_keyMap[SDLK_LEFT] = RETRO_DEVICE_ID_JOYPAD_LEFT;
    m_keyMap[SDLK_RIGHT] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
}

void InputHandler::update() {
    // 1. Clear digital button state and analog state.
    for (int i = 0; i < 16; ++i) {
        m_buttonStates[i] = false;
    }
    m_analogLeftX = 0;
    m_analogLeftY = 0;
    m_analogRightX = 0;
    m_analogRightY = 0;
    m_analogL2 = 0; 
    m_analogR2 = 0; 

    // 2. Update input state from all sources.
    updateGamepad();
    updateKeyboard(); 
}

// FIX: This function definition is required to match the declaration in the header
void InputHandler::poll() {
    // This is the implementation for retro_input_poll_t.
    // We already update the state in InputHandler::update(), so this function is empty,
    // but it must exist to satisfy the linker and the core's callback.
}

void InputHandler::handleEvent(const SDL_Event& event) {
    // Handle controller connection/disconnection
    switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            // Only grab the controller if we don't already have one
            if (!m_controller) {
                m_controller = SDL_GameControllerOpen(event.cdevice.which);
                if (m_controller) {
                    std::cout << "Game controller connected: " 
                             << SDL_GameControllerName(m_controller) << std::endl;
                }
            }
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            // Check if the disconnected controller is the one we are using
            if (m_controller && event.cdevice.which == SDL_JoystickInstanceID(
                SDL_GameControllerGetJoystick(m_controller))) {
                SDL_GameControllerClose(m_controller);
                m_controller = nullptr;
                std::cout << "Game controller disconnected" << std::endl;
            }
            break;
            
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_pointerPressed = true;
                // Update pointer position when clicked
                m_pointerX = event.button.x;
                m_pointerY = event.button.y;
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_pointerPressed = false;
            }
            break;
            
        case SDL_MOUSEMOTION:
            // Store raw mouse coordinates - will be converted to libretro coordinates in getInputState
            m_pointerX = event.motion.x;
            m_pointerY = event.motion.y;
            break;
    }
    
    // Check for quit event
    if (event.type == SDL_QUIT || 
       (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
        m_quit = true;
    }
}

void InputHandler::updateKeyboard() {
    const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

    // 1. Set digital button states based on m_keyMap (Keyboard)
    for (const auto& pair : m_keyMap) {
        if (keyState[SDL_GetScancodeFromKey(pair.first)]) {
            unsigned retro_id = pair.second;
            if (retro_id < 16) {
                m_buttonStates[retro_id] = true;
            }
        }
    }

    // 2. Set Analog stick values based on common WASD/Arrow key inputs (Keyboard)
    const int16_t MAX_AXIS = 32767;

    // Left Analog Stick (WASD/Arrows for main movement)
    if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT]) m_analogLeftX = -MAX_AXIS;
    else if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT]) m_analogLeftX = MAX_AXIS;
    
    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP]) m_analogLeftY = -MAX_AXIS;
    else if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN]) m_analogLeftY = MAX_AXIS;
}

void InputHandler::updateGamepad() {
    if (!m_controller) return;

    // 1. Map digital controller buttons
    if (RETRO_DEVICE_ID_JOYPAD_A < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_A] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_A);
    if (RETRO_DEVICE_ID_JOYPAD_B < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_B] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_B);
    if (RETRO_DEVICE_ID_JOYPAD_X < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_X] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_X);
    if (RETRO_DEVICE_ID_JOYPAD_Y < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_Y] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_Y);
    if (RETRO_DEVICE_ID_JOYPAD_L < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_L] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    if (RETRO_DEVICE_ID_JOYPAD_R < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_R] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    if (RETRO_DEVICE_ID_JOYPAD_START < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_START] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_START);
    if (RETRO_DEVICE_ID_JOYPAD_SELECT < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_SELECT] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_BACK);
    
    // D-Pad buttons
    if (RETRO_DEVICE_ID_JOYPAD_UP < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_UP] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
    if (RETRO_DEVICE_ID_JOYPAD_DOWN < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_DOWN] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    if (RETRO_DEVICE_ID_JOYPAD_LEFT < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_LEFT] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    if (RETRO_DEVICE_ID_JOYPAD_RIGHT < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_RIGHT] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        
    // L3, R3 (stick clicks)
    if (RETRO_DEVICE_ID_JOYPAD_L3 < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_L3] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
    if (RETRO_DEVICE_ID_JOYPAD_R3 < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_R3] |= SDL_GameControllerGetButton(m_controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
        
    // L2, R2 (Triggers as digital buttons)
    if (RETRO_DEVICE_ID_JOYPAD_L2 < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_L2] |= (SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > ANALOG_DEADZONE);
    if (RETRO_DEVICE_ID_JOYPAD_R2 < 16)
        m_buttonStates[RETRO_DEVICE_ID_JOYPAD_R2] |= (SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > ANALOG_DEADZONE);
        
    // 2. Poll and store Analog Stick Axes
    m_analogLeftX = processAnalogAxis(SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_LEFTX));
    m_analogLeftY = processAnalogAxis(SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_LEFTY));
    m_analogRightX = processAnalogAxis(SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_RIGHTX));
    m_analogRightY = processAnalogAxis(SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_RIGHTY));

    // 3. Poll and store Analog Trigger Axes (0 to 32767)
    m_analogL2 = SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    m_analogR2 = SDL_GameControllerGetAxis(m_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
}

int16_t InputHandler::getInputState(unsigned port, unsigned device, unsigned index, unsigned id) {
    // Only support player 1 for this handler
    if (port != 0) return 0;
    
    // Handle digital joypad input (RETRO_DEVICE_JOYPAD)
    if (device == RETRO_DEVICE_JOYPAD) {
        if (id < 16) {
            return m_buttonStates[id] ? 1 : 0;
        }
        return 0;
    }
    
    // Handle analog stick input (RETRO_DEVICE_ANALOG)
    if (device == RETRO_DEVICE_ANALOG) {
        // Left Analog Stick (RETRO_DEVICE_INDEX_ANALOG_LEFT = 0)
        if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT) {
            if (id == RETRO_DEVICE_ID_ANALOG_X) {
                return m_analogLeftX;
            } else if (id == RETRO_DEVICE_ID_ANALOG_Y) {
                return m_analogLeftY;
            }
            // Expose L2/R2 axes using the button IDs, but in the analog context.
            else if (id == RETRO_DEVICE_ID_JOYPAD_L2) {
                 // L2 axis (0 to 32767)
                return m_analogL2; 
            }
        } 
        // Right Analog Stick (RETRO_DEVICE_INDEX_ANALOG_RIGHT = 1)
        else if (index == RETRO_DEVICE_INDEX_ANALOG_RIGHT) {
            if (id == RETRO_DEVICE_ID_ANALOG_X) {
                return m_analogRightX;
            } else if (id == RETRO_DEVICE_ID_ANALOG_Y) {
                return m_analogRightY;
            }
            else if (id == RETRO_DEVICE_ID_JOYPAD_R2) {
                // R2 axis (0 to 32767)
                return m_analogR2; 
            }
        }
    }
    
    // Handle pointer/touchscreen input (RETRO_DEVICE_POINTER)
    if (device == RETRO_DEVICE_POINTER) {
        if (id == RETRO_DEVICE_ID_POINTER_X) {
            // Convert screen coordinates to libretro pointer coordinates (-32767 to 32767)
            // Need to get window size to normalize
            extern SDL_Window* g_gameWindow;
            if (g_gameWindow) {
                int windowWidth, windowHeight;
                SDL_GetWindowSize(g_gameWindow, &windowWidth, &windowHeight);
                // Normalize to -32767 to 32767 range
                int16_t result = (int16_t)(((m_pointerX * 65535) / windowWidth) - 32768);
                static int logCount = 0;
                if (m_pointerPressed && logCount++ % 60 == 0) {
                    std::cout << "[POINTER] X: raw=" << m_pointerX << " window=" << windowWidth 
                              << " result=" << result << std::endl;
                }
                return result;
            }
            return 0;
        } else if (id == RETRO_DEVICE_ID_POINTER_Y) {
            extern SDL_Window* g_gameWindow;
            if (g_gameWindow) {
                int windowWidth, windowHeight;
                SDL_GetWindowSize(g_gameWindow, &windowWidth, &windowHeight);
                int16_t result = (int16_t)(((m_pointerY * 65535) / windowHeight) - 32768);
                return result;
            }
            return 0;
        } else if (id == RETRO_DEVICE_ID_POINTER_PRESSED) {
            static int logCount = 0;
            if (m_pointerPressed && logCount++ % 60 == 0) {
                std::cout << "[POINTER] Pressed at (" << m_pointerX << ", " << m_pointerY << ")" << std::endl;
            }
            return m_pointerPressed ? 1 : 0;
        }
    }
    
    return 0;
}