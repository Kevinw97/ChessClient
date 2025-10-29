#pragma once
#include "chess.h"

namespace chess_client {
  class AudioHandler {
  private:
    SDL_AudioStream* m_Stream;

    void playAudioFile(const char* path);
  public:
    AudioHandler();
    ~AudioHandler();
    void playMoveSound();
    void playCaptureSound();
    void init();
  };
}  // namespace chess_client