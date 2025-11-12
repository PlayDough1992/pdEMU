#include "video_renderer_gl.h"
#include <SDL2/SDL_opengl.h>
#include <iostream>
#include <cstring>

// Define missing GL constants if not available
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

VideoRendererGL::VideoRendererGL()
    : m_window(nullptr)
    , m_glContext(nullptr)
    , m_texture(0)
    , m_framebuffer(0)
    , m_baseWidth(0)
    , m_baseHeight(0)
    , m_aspectRatio(1.0f)
    , m_internalScale(2)
    , m_linearFilter(true)
    , m_initialized(false)
{
}

VideoRendererGL::~VideoRendererGL() {
    shutdown();
}

bool VideoRendererGL::init(SDL_Window* window, int baseWidth, int baseHeight, float aspectRatio) {
    m_window = window;
    m_baseWidth = baseWidth;
    m_baseHeight = baseHeight;
    m_aspectRatio = aspectRatio;
    
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    // Create OpenGL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Enable VSync
    SDL_GL_SetSwapInterval(1);
    
    // Initialize OpenGL settings
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Generate texture
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_linearFilter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_linearFilter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    updateViewport();
    
    m_initialized = true;
    std::cout << "OpenGL renderer initialized: " << baseWidth << "x" << baseHeight << std::endl;
    
    return true;
}

void VideoRendererGL::shutdown() {
    if (!m_initialized) return;
    
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    
    m_initialized = false;
}

void VideoRendererGL::updateFrame(const void* data, unsigned width, unsigned height, size_t pitch) {
    if (!m_initialized || !data) return;
    
    glBindTexture(GL_TEXTURE_2D, m_texture);
    
    // Handle pitch (row stride) - libretro gives us XRGB8888 format
    if (pitch == width * 4) {
        // No padding, can upload directly
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
    } else {
        // Has padding, need to handle row by row
        glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }
}

void VideoRendererGL::render() {
    if (!m_initialized) return;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_debugMode) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "[OpenGL Debug] glClear error: 0x" << std::hex << err << std::dec << std::endl;
        }
    }

    glBindTexture(GL_TEXTURE_2D, m_texture);
    if (m_debugMode) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "[OpenGL Debug] glBindTexture error: 0x" << std::hex << err << std::dec << std::endl;
        }
    }

    // Draw a textured quad
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
    glEnd();
    if (m_debugMode) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "[OpenGL Debug] glBegin/glEnd error: 0x" << std::hex << err << std::dec << std::endl;
        }
    }
}

void VideoRendererGL::present() {
    if (!m_initialized) return;
    if (m_debugMode) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "[OpenGL Debug] (before swap) error: 0x" << std::hex << err << std::dec << std::endl;
        }
    }
    SDL_GL_SwapWindow(m_window);
    if (m_debugMode) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "[OpenGL Debug] (after swap) error: 0x" << std::hex << err << std::dec << std::endl;
        }
    }
}

void VideoRendererGL::setInternalScale(int scale) {
    m_internalScale = scale;
    updateViewport();
}

void VideoRendererGL::setLinearFilter(bool enabled) {
    m_linearFilter = enabled;
    
    if (m_initialized && m_texture) {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_linearFilter ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_linearFilter ? GL_LINEAR : GL_NEAREST);
    }
}

void VideoRendererGL::makeCurrent() {
    if (m_glContext && m_window) {
        SDL_GL_MakeCurrent(m_window, m_glContext);
    }
}

void VideoRendererGL::updateViewport() {
    if (!m_initialized) return;
    
    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    
    // Calculate viewport to maintain aspect ratio
    float windowAspect = (float)windowWidth / windowHeight;
    int viewportWidth, viewportHeight, viewportX, viewportY;
    
    if (windowAspect > m_aspectRatio) {
        // Window is wider than game aspect ratio
        viewportHeight = windowHeight;
        viewportWidth = (int)(windowHeight * m_aspectRatio);
        viewportX = (windowWidth - viewportWidth) / 2;
        viewportY = 0;
    } else {
        // Window is taller than game aspect ratio
        viewportWidth = windowWidth;
        viewportHeight = (int)(windowWidth / m_aspectRatio);
        viewportX = 0;
        viewportY = (windowHeight - viewportHeight) / 2;
    }
    
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
}
