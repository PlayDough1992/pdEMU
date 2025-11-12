#include "audio_renderer.h"
#include <iostream>
#include <algorithm>

AudioRenderer::AudioRenderer()
    : m_audioDevice(0)
    , m_initialized(false)
{
}

AudioRenderer::~AudioRenderer() {
    shutdown();
}

bool AudioRenderer::init(double sampleRate) {
    if (m_initialized) {
        std::cerr << "Audio renderer already initialized" << std::endl;
        return false;
    }

    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);

    desired.freq = static_cast<int>(sampleRate);
    desired.format = AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples = 2048;  // Increased from 1024 to reduce audio hitches
    desired.callback = audioCallback;
    desired.userdata = this;

    m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    
    if (m_audioDevice == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }

    // Reserve space for larger audio buffer to prevent underruns
    m_audioBuffer.reserve(obtained.samples * 8);

    // Start audio playback
    SDL_PauseAudioDevice(m_audioDevice, 0);

    m_initialized = true;
    return true;
}

void AudioRenderer::shutdown() {
    if (m_initialized) {
        if (m_audioDevice) {
            SDL_CloseAudioDevice(m_audioDevice);
            m_audioDevice = 0;
        }
        m_initialized = false;
    }
}

void AudioRenderer::pushSample(int16_t left, int16_t right) {
    std::lock_guard<std::mutex> lock(m_audioMutex);
    m_audioBuffer.push_back(left);
    m_audioBuffer.push_back(right);
}

void AudioRenderer::pushSamples(const int16_t* data, size_t frames) {
    std::lock_guard<std::mutex> lock(m_audioMutex);
    
    // Each frame is 2 samples (left and right)
    size_t sampleCount = frames * 2;
    m_audioBuffer.insert(m_audioBuffer.end(), data, data + sampleCount);
}

void AudioRenderer::audioCallback(void* userdata, uint8_t* stream, int len) {
    AudioRenderer* renderer = static_cast<AudioRenderer*>(userdata);
    if (!renderer) return;

    std::lock_guard<std::mutex> lock(renderer->m_audioMutex);

    int samples = len / sizeof(int16_t);
    int16_t* output = reinterpret_cast<int16_t*>(stream);

    if (renderer->m_audioBuffer.size() >= static_cast<size_t>(samples)) {
        // Copy available samples
        std::copy(
            renderer->m_audioBuffer.begin(),
            renderer->m_audioBuffer.begin() + samples,
            output
        );
        // Remove consumed samples
        renderer->m_audioBuffer.erase(
            renderer->m_audioBuffer.begin(),
            renderer->m_audioBuffer.begin() + samples
        );
    } else {
        // Not enough data, output silence
        std::fill(output, output + samples, 0);
        renderer->m_audioBuffer.clear();
    }
}
