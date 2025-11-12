#include "splash_screen.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <thread>
#include <chrono>

SplashScreen::SplashScreen()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_logoTexture(nullptr)
    , m_logoSurface(nullptr)
    , m_initialized(false)
    , m_showing(false)
    , m_windowWidth(1024)
    , m_windowHeight(768)
{
}

SplashScreen::~SplashScreen() {
    shutdown();
}

bool SplashScreen::init(SDL_Window* window, SDL_Renderer* renderer) {
    if (m_initialized) {
        return true;
    }
    
    m_window = window;
    m_renderer = renderer;
    
    // Get window size
    SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
    
    // Initialize SDL_image for PNG loading
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
    
    // Load logo
    if (!loadLogo("LOGO/pdEMU_LOGO.png")) {
        std::cerr << "Warning: Could not load splash logo" << std::endl;
        // Don't fail initialization, just continue without logo
    }
    
    m_initialized = true;
    return true;
}

void SplashScreen::shutdown() {
    if (m_logoTexture) {
        SDL_DestroyTexture(m_logoTexture);
        m_logoTexture = nullptr;
    }
    
    if (m_logoSurface) {
        SDL_FreeSurface(m_logoSurface);
        m_logoSurface = nullptr;
    }
    
    IMG_Quit();
    m_initialized = false;
}

bool SplashScreen::loadLogo(const std::string& logoPath) {
    m_logoSurface = IMG_Load(logoPath.c_str());
    if (!m_logoSurface) {
        std::cerr << "Failed to load logo image: " << IMG_GetError() << std::endl;
        return false;
    }
    
    m_logoTexture = SDL_CreateTextureFromSurface(m_renderer, m_logoSurface);
    if (!m_logoTexture) {
        std::cerr << "Failed to create texture from logo: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(m_logoSurface);
        m_logoSurface = nullptr;
        return false;
    }
    
    return true;
}

void SplashScreen::renderFrame(const std::string& statusText) {
    if (!m_initialized || !m_renderer) {
        return;
    }
    
    // Clear screen to black
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    
    // Draw logo if available
    if (m_logoTexture) {
        int logoW, logoH;
        SDL_QueryTexture(m_logoTexture, nullptr, nullptr, &logoW, &logoH);
        
        // Center logo on screen
        int x = (m_windowWidth - logoW) / 2;
        int y = (m_windowHeight - logoH) / 2 - 50; // Offset up a bit for status text
        
        SDL_Rect destRect = { x, y, logoW, logoH };
        SDL_RenderCopy(m_renderer, m_logoTexture, nullptr, &destRect);
    }
    
    // Draw status text (if we had a font rendering system)
    // For now, just the logo is fine
    // TODO: Could add SDL_ttf for text rendering
    
    SDL_RenderPresent(m_renderer);
}

void SplashScreen::show(int durationMs, const std::string& statusText) {
    if (!m_initialized) {
        return;
    }
    
    m_showing = true;
    
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::milliseconds(durationMs);
    
    // Show splash for specified duration
    while (std::chrono::steady_clock::now() < endTime) {
        // Process events to keep window responsive
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_showing = false;
                return;
            }
            // Allow ESC to skip splash
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                m_showing = false;
                return;
            }
        }
        
        renderFrame(statusText);
        SDL_Delay(16); // ~60 FPS
    }
    
    m_showing = false;
}

void SplashScreen::showWithStatus(const std::string& statusText) {
    if (!m_initialized) {
        return;
    }
    
    m_showing = true;
    renderFrame(statusText);
}

void SplashScreen::updateStatus(const std::string& statusText) {
    if (!m_initialized || !m_showing) {
        return;
    }
    
    // Process events to keep responsive
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || 
            (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
            m_showing = false;
            return;
        }
    }
    
    renderFrame(statusText);
}

void SplashScreen::hide() {
    m_showing = false;
}
