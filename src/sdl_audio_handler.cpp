#include "sdl_audio_handler.h"

namespace chess_client {
  AudioHandler::AudioHandler() : m_Stream(nullptr) {};

  AudioHandler::~AudioHandler() {
    if (m_Stream) {
      SDL_DestroyAudioStream(m_Stream);
    }
  }

  void AudioHandler::playAudioFile(const char* filepath) {
    if (!m_Stream) {
      std::cerr << "Audio stream not initialized" << std::endl;
      return;
    }

    // Load the WAV file
    SDL_AudioSpec spec;
    Uint8* audio_buf = nullptr;
    Uint32 audio_len = 0;

    if (!SDL_LoadWAV(filepath, &spec, &audio_buf, &audio_len)) {
      std::cerr << "Failed to load audio file " << filepath << ": " << SDL_GetError() << std::endl;
      return;
    }

    // Put the audio data into the stream
    if (!SDL_PutAudioStreamData(m_Stream, audio_buf, audio_len)) {
      std::cerr << "Failed to put audio data into stream: " << SDL_GetError() << std::endl;
    }

    // Free the audio buffer
    SDL_free(audio_buf);
  }

  void AudioHandler::playMoveSound() {
    playAudioFile("res/move.wav");
  };

  void AudioHandler::playCaptureSound() {
    playAudioFile("res/capture.wav");
  };

  void AudioHandler::init() {
    SDL_AudioSpec audio_spec;
    audio_spec.format = SDL_AUDIO_S16;
    audio_spec.channels = 2;
    audio_spec.freq = 48000;

    m_Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec, NULL, NULL);
    if (!m_Stream) {
      std::cerr << "Failed to open audio stream: " << SDL_GetError() << std::endl;
      return;
    }
    SDL_ResumeAudioStreamDevice(m_Stream);
  }
}  // namespace chess_client