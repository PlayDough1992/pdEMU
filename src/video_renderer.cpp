#include "video_renderer.h"
#include <iostream>
#include <cstring>

VideoRenderer::VideoRenderer()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_texture(nullptr)
    , m_scaledTexture(nullptr)
    , m_initialized(false)
    , m_width(0)
    , m_height(0)
    , m_internalScale(1)
    , m_linearFilter(true)
    , m_pixelFormat(PIXEL_FORMAT_RGB565)
    , m_frameBuffer(nullptr)
    , m_frameBufferSize(0)
{
}

VideoRenderer::~VideoRenderer() {
    shutdown();
}

bool VideoRenderer::init(const char* title, int width, int height) {
    if (m_initialized) {
        std::cerr << "Video renderer already initialized" << std::endl;
        return false;
    }

    m_width = width;
    m_height = height;

    // Create window
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    m_renderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        return false;
    }

    // Set logical size for aspect ratio preservation
    SDL_RenderSetLogicalSize(m_renderer, width, height);
    const char* filterHint = m_linearFilter ? "linear" : "nearest";
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filterHint);

    m_initialized = true;
    return true;
}

void VideoRenderer::shutdown() {
    if (m_frameBuffer) {
        delete[] m_frameBuffer;
        m_frameBuffer = nullptr;
        m_frameBufferSize = 0;
    }

    if (m_scaledTexture) {
        SDL_DestroyTexture(m_scaledTexture);
        m_scaledTexture = nullptr;
    }

    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    m_initialized = false;
}

void VideoRenderer::render(const void* data, unsigned width, unsigned height, size_t pitch) {
    if (!m_initialized || !data) {
        return;
    }

    // Create or recreate texture if dimensions changed
    if (!m_texture || m_width != static_cast<int>(width) || m_height != static_cast<int>(height)) {
        if (m_texture) {
            SDL_DestroyTexture(m_texture);
        }

        m_texture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height
        );

        if (!m_texture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            return;
        }
        
        // Allocate conversion buffer
        size_t bufferSize = width * height;
        if (m_frameBufferSize < bufferSize) {
            if (m_frameBuffer) {
                delete[] m_frameBuffer;
            }
            m_frameBuffer = new uint32_t[bufferSize];
            m_frameBufferSize = bufferSize;
        }
    }

    // Update texture with frame data
    void* pixels;
    int texturePitch;
    
    if (SDL_LockTexture(m_texture, nullptr, &pixels, &texturePitch) == 0) {
        uint32_t* dst = static_cast<uint32_t*>(pixels);
        
        // Convert based on pixel format
        if (m_pixelFormat == PIXEL_FORMAT_RGB565) {
            convertRGB565ToXRGB8888(static_cast<const uint16_t*>(data), dst, width, height, pitch);
        } else if (m_pixelFormat == PIXEL_FORMAT_0RGB1555) {
            convert0RGB1555ToXRGB8888(static_cast<const uint16_t*>(data), dst, width, height, pitch);
        } else {
            // Assume XRGB8888 or copy directly
            const uint8_t* src = static_cast<const uint8_t*>(data);
            uint8_t* dstBytes = static_cast<uint8_t*>(pixels);
            for (unsigned y = 0; y < height; ++y) {
                memcpy(dstBytes + y * texturePitch, src + y * pitch, width * 4);
            }
        }

        SDL_UnlockTexture(m_texture);
    }

    // Store dimensions for present()
    m_width = width;
    m_height = height;
}

void VideoRenderer::convertRGB565ToXRGB8888(const uint16_t* src, uint32_t* dst, unsigned width, unsigned height, size_t srcPitch) {
    size_t srcPitchPixels = srcPitch / 2;
    
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            uint16_t pixel = src[y * srcPitchPixels + x];
            
            // Extract RGB565 components
            uint8_t r = ((pixel >> 11) & 0x1F) << 3;
            uint8_t g = ((pixel >> 5) & 0x3F) << 2;
            uint8_t b = (pixel & 0x1F) << 3;
            
            // Expand to full 8-bit range
            r |= r >> 5;
            g |= g >> 6;
            b |= b >> 5;
            
            // Pack into XRGB8888
            dst[y * width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }
}

void VideoRenderer::convert0RGB1555ToXRGB8888(const uint16_t* src, uint32_t* dst, unsigned width, unsigned height, size_t srcPitch) {
    size_t srcPitchPixels = srcPitch / 2;
    
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            uint16_t pixel = src[y * srcPitchPixels + x];
            
            // Extract 0RGB1555 components
            uint8_t r = ((pixel >> 10) & 0x1F) << 3;
            uint8_t g = ((pixel >> 5) & 0x1F) << 3;
            uint8_t b = (pixel & 0x1F) << 3;
            
            // Expand to full 8-bit range
            r |= r >> 5;
            g |= g >> 5;
            b |= b >> 5;
            
            // Pack into XRGB8888
            dst[y * width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }
}

void VideoRenderer::clear() {
    if (m_initialized && m_renderer) {
        SDL_RenderClear(m_renderer);
    }
}

void VideoRenderer::present() {
    if (m_initialized && m_renderer && m_texture) {
        // Clear and render the texture
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }
}

void VideoRenderer::applyInternalScaling(const uint32_t* src, unsigned width, unsigned height) {
    if (m_internalScale == 1) {
        // No scaling, use source directly
        return;
    }
    
    // Create scaled texture if needed
    unsigned scaledWidth = width * m_internalScale;
    unsigned scaledHeight = height * m_internalScale;
    
    if (!m_scaledTexture) {
        m_scaledTexture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            scaledWidth,
            scaledHeight
        );
    }
    
    if (!m_scaledTexture) return;
    
    // Lock and scale
    void* pixels;
    int pitch;
    if (SDL_LockTexture(m_scaledTexture, nullptr, &pixels, &pitch) == 0) {
        uint32_t* dst = static_cast<uint32_t*>(pixels);
        
        // Simple nearest-neighbor scaling
        for (unsigned y = 0; y < scaledHeight; ++y) {
            unsigned srcY = y / m_internalScale;
            for (unsigned x = 0; x < scaledWidth; ++x) {
                unsigned srcX = x / m_internalScale;
                dst[y * (pitch / 4) + x] = src[srcY * width + srcX];
            }
        }
        
        SDL_UnlockTexture(m_scaledTexture);
    }
}

