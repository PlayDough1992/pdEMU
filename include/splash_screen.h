#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <SDL2/SDL.h>
#include <string>

class SplashScreen {
public:
    SplashScreen();
    ~SplashScreen();
    
    bool init(SDL_Window* window, SDL_Renderer* renderer);
    void shutdown();
    
    // Display splash screen for specified duration (milliseconds)
    void show(int durationMs = 2000, const std::string& statusText = "");
    
    // Show with loading text that can be updated
    void showWithStatus(const std::string& statusText);
    void updateStatus(const std::string& statusText);
    void hide();
    
    bool isShowing() const { return m_showing; }
    
private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_logoTexture;
    SDL_Surface* m_logoSurface;
    
    bool m_initialized;
    bool m_showing;
    
    int m_windowWidth;
    int m_windowHeight;
    
    bool loadLogo(const std::string& logoPath);
    void renderFrame(const std::string& statusText = "");
};

#endif // SPLASH_SCREEN_H
