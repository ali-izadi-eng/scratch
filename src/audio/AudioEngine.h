#pragma once

#include <SDL.h>

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// Minimal audio engine:
// - WAV only (via SDL_LoadWAV)
// - Mixes multiple playing sounds
// - Supports per-play volume and a simple pitch control via resampling
class AudioEngine {
public:
  struct PlayResult {
    int handle{-1};
    float durationSec{0.0f};
  };

  static AudioEngine& instance();

  bool init();
  void shutdown();

  // volume: 0..1
  // pitchSemitones: e.g. +12 = one octave up
  PlayResult playWav(const std::string& filePath, float volume, float pitchSemitones);
  void stopAll();

  SDL_AudioSpec deviceSpec() const { return haveSpec_; }

private:
  AudioEngine() = default;
  AudioEngine(const AudioEngine&) = delete;
  AudioEngine& operator=(const AudioEngine&) = delete;

  struct SoundData {
    std::vector<uint8_t> pcm; // device format (AUDIO_S16LSB, stereo)
    uint32_t frames{0};       // number of stereo frames
  };

  struct Channel {
    int handle{0};
    const SoundData* snd{nullptr};
    double posFrames{0.0};
    double rate{1.0};
    float volume{1.0f};
    bool finished{false};
  };

  static void audioCallback(void* userdata, Uint8* stream, int len);
  void mix(Uint8* stream, int len);

  const SoundData* loadCachedWavLocked(const std::string& filePath);

private:
  SDL_AudioDeviceID dev_{0};
  SDL_AudioSpec wantSpec_{};
  SDL_AudioSpec haveSpec_{};

  std::mutex mtx_;
  std::unordered_map<std::string, SoundData> cache_; // by filePath
  std::vector<Channel> channels_;

  int nextHandle_{1};
  bool initialized_{false};
};
