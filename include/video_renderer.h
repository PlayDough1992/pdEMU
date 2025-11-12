#ifndef VIDEO_RENDERER_H
#define VIDEO_RENDERER_H

#include <SDL2/SDL.h>
#include <cstdint>

enum PixelFormat {
    PIXEL_FORMAT_UNKNOWN,
    PIXEL_FORMAT_RGB565,
    PIXEL_FORMAT_XRGB8888,
    PIXEL_FORMAT_0RGB1555
};

class VideoRenderer {
public:
    VideoRenderer();
    ~VideoRenderer();

    bool init(const char* title, int width, int height);
    void shutdown();
    
    void render(const void* data, unsigned width, unsigned height, size_t pitch);
    void setPixelFormat(PixelFormat format) { m_pixelFormat = format; }
    void setInternalScale(int scale) { m_internalScale = scale; }
    void setLinearFilter(bool enable) { m_linearFilter = enable; }
    void clear();
    void present();
    
    SDL_Window* getWindow() { return m_window; }
    SDL_Renderer* getRenderer() { return m_renderer; }
    bool isInitialized() const { return m_initialized; }
    
private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_texture;
    SDL_Texture* m_scaledTexture;
    
    bool m_initialized;
    int m_width;
    int m_height;
    int m_internalScale;
    bool m_linearFilter;
    PixelFormat m_pixelFormat;
    
    // Frame buffer for pixel format conversion if needed
    uint32_t* m_frameBuffer;
    size_t m_frameBufferSize;
    
    void convertRGB565ToXRGB8888(const uint16_t* src, uint32_t* dst, unsigned width, unsigned height, size_t srcPitch);
    void convert0RGB1555ToXRGB8888(const uint16_t* src, uint32_t* dst, unsigned width, unsigned height, size_t srcPitch);
    void applyInternalScaling(const uint32_t* src, unsigned width, unsigned height);
};

#endif // VIDEO_RENDERER_H
