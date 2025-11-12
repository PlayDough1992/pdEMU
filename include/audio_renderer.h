#ifndef AUDIO_RENDERER_H
#define AUDIO_RENDERER_H

#include <SDL2/SDL.h>
#include <vector>
#include <mutex>

class AudioRenderer {
public:
    AudioRenderer();
    ~AudioRenderer();

    bool init(double sampleRate);
    void shutdown();
    
    void pushSample(int16_t left, int16_t right);
    void pushSamples(const int16_t* data, size_t frames);
    
    bool isInitialized() const { return m_initialized; }

private:
    SDL_AudioDeviceID m_audioDevice;
    bool m_initialized;
    
    std::vector<int16_t> m_audioBuffer;
    std::mutex m_audioMutex;
    
    static void audioCallback(void* userdata, uint8_t* stream, int len);
};

#endif // AUDIO_RENDERER_H
