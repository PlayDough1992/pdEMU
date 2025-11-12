#ifndef VIDEO_RENDERER_GL_H
#define VIDEO_RENDERER_GL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cstdint>

class VideoRendererGL {
public:
    VideoRendererGL();
    ~VideoRendererGL();
    
    bool init(SDL_Window* window, int baseWidth, int baseHeight, float aspectRatio);
    void shutdown();
    
    void updateFrame(const void* data, unsigned width, unsigned height, size_t pitch);
    void render();
    void present();
    
    void setInternalScale(int scale);
    void setLinearFilter(bool enabled);
    void makeCurrent();
    
    int getInternalScale() const { return m_internalScale; }
    SDL_Window* getWindow() const { return m_window; }
    
    // Debug mode: skip/log OpenGL errors
    void setDebugMode(bool debug) { m_debugMode = debug; }
    bool getDebugMode() const { return m_debugMode; }

private:
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    bool m_debugMode = false;
    
    GLuint m_texture;
    GLuint m_framebuffer;
    
    int m_baseWidth;
    int m_baseHeight;
    float m_aspectRatio;
    int m_internalScale;
    bool m_linearFilter;
    bool m_initialized;
    
    void updateViewport();
};

#endif // VIDEO_RENDERER_GL_H
