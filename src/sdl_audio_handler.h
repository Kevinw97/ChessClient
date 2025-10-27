#pragma once
#include "chess.h"

namespace chess_client {
  class AudioHandler {
  private:
    SDL_AudioStream* m_Stream;

    void PlayAudioFile(const char* path);
  public:
    AudioHandler();
    ~AudioHandler();
    void playMoveSound();
    void playCaptureSound();
  };
}  // namespace chess_client