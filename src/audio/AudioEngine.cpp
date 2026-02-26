#include "audio/AudioEngine.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "core/Logger.h"

AudioEngine& AudioEngine::instance() {
  static AudioEngine g;
  return g;
}

bool AudioEngine::init() {
  if (initialized_) return true;

  SDL_zero(wantSpec_);
  wantSpec_.freq = 44100;
  wantSpec_.format = AUDIO_S16LSB;
  wantSpec_.channels = 2;
  wantSpec_.samples = 1024;
  wantSpec_.callback = &AudioEngine::audioCallback;
  wantSpec_.userdata = this;

  dev_ = SDL_OpenAudioDevice(nullptr, 0, &wantSpec_, &haveSpec_, 0);
  if (!dev_) {
    Logger::warn("Audio", std::string("SDL_OpenAudioDevice failed: ") + SDL_GetError());
    return false;
  }

  SDL_PauseAudioDevice(dev_, 0);
  initialized_ = true;
  Logger::info("Audio", "AudioEngine initialized");
  return true;
}

void AudioEngine::shutdown() {
  if (!initialized_) return;
  stopAll();
  if (dev_) {
    SDL_CloseAudioDevice(dev_);
    dev_ = 0;
  }
  {
    std::lock_guard<std::mutex> lk(mtx_);
    cache_.clear();
  }
  initialized_ = false;
  Logger::info("Audio", "AudioEngine shutdown");
}

void AudioEngine::stopAll() {
  std::lock_guard<std::mutex> lk(mtx_);
  channels_.clear();
}

static inline float clamp01(float v) {
  if (v < 0.0f) return 0.0f;
  if (v > 1.0f) return 1.0f;
  return v;
}

const AudioEngine::SoundData* AudioEngine::loadCachedWavLocked(const std::string& filePath) {
  auto it = cache_.find(filePath);
  if (it != cache_.end()) return &it->second;

  SDL_AudioSpec srcSpec{};
  Uint8* srcBuf = nullptr;
  Uint32 srcLen = 0;
  if (!SDL_LoadWAV(filePath.c_str(), &srcSpec, &srcBuf, &srcLen)) {
    Logger::warn("Audio", std::string("SDL_LoadWAV failed: ") + SDL_GetError());
    return nullptr;
  }

  // Convert to device format
  SDL_AudioCVT cvt;
  if (SDL_BuildAudioCVT(&cvt,
                        srcSpec.format, srcSpec.channels, srcSpec.freq,
                        haveSpec_.format, haveSpec_.channels, haveSpec_.freq) < 0) {
    SDL_FreeWAV(srcBuf);
    Logger::warn("Audio", std::string("SDL_BuildAudioCVT failed: ") + SDL_GetError());
    return nullptr;
  }

  std::vector<uint8_t> converted;
  if (cvt.needed) {
    converted.resize(srcLen * cvt.len_mult);
    std::memcpy(converted.data(), srcBuf, srcLen);
    cvt.buf = converted.data();
    cvt.len = (int)srcLen;
    if (SDL_ConvertAudio(&cvt) < 0) {
      SDL_FreeWAV(srcBuf);
      Logger::warn("Audio", std::string("SDL_ConvertAudio failed: ") + SDL_GetError());
      return nullptr;
    }
    converted.resize((size_t)cvt.len_cvt);
  } else {
    converted.assign(srcBuf, srcBuf + srcLen);
  }

  SDL_FreeWAV(srcBuf);

  // device format is expected to be S16 stereo
  if (haveSpec_.format != AUDIO_S16LSB || haveSpec_.channels != 2) {
    Logger::warn("Audio", "Device format is not AUDIO_S16LSB stereo; mixer assumes it.");
  }

  SoundData sd;
  sd.pcm = std::move(converted);
  sd.frames = (uint32_t)(sd.pcm.size() / (sizeof(int16_t) * 2));

  auto [insIt, _] = cache_.emplace(filePath, std::move(sd));
  return &insIt->second;
}

AudioEngine::PlayResult AudioEngine::playWav(const std::string& filePath, float volume, float pitchSemitones) {
  PlayResult pr;
  if (!initialized_) {
    Logger::warn("Audio", "playWav called but AudioEngine not initialized.");
    return pr;
  }

  volume = clamp01(volume);
  // semitone to rate: 2^(n/12)
  double rate = std::pow(2.0, (double)pitchSemitones / 12.0);
  if (rate < 0.25) rate = 0.25;
  if (rate > 4.0)  rate = 4.0;

  std::lock_guard<std::mutex> lk(mtx_);
  const SoundData* sd = loadCachedWavLocked(filePath);
  if (!sd || sd->frames == 0) return pr;

  Channel ch;
  ch.handle = nextHandle_++;
  ch.snd = sd;
  ch.posFrames = 0.0;
  ch.rate = rate;
  ch.volume = volume;
  ch.finished = false;
  channels_.push_back(ch);

  pr.handle = ch.handle;
  pr.durationSec = (float)((double)sd->frames / (double)haveSpec_.freq / rate);
  return pr;
}

void AudioEngine::audioCallback(void* userdata, Uint8* stream, int len) {
  ((AudioEngine*)userdata)->mix(stream, len);
}

static inline int16_t clampS16(int v) {
  if (v < -32768) return -32768;
  if (v > 32767) return 32767;
  return (int16_t)v;
}

void AudioEngine::mix(Uint8* stream, int len) {
  // Clear output
  std::memset(stream, 0, (size_t)len);
  if (len <= 0) return;

  const int outFrames = len / (int)(sizeof(int16_t) * 2);
  if (outFrames <= 0) return;

  std::lock_guard<std::mutex> lk(mtx_);
  if (channels_.empty()) return;

  int16_t* out = (int16_t*)stream;

  for (auto& ch : channels_) {
    if (!ch.snd || ch.finished) continue;
    const int16_t* src = (const int16_t*)ch.snd->pcm.data();
    const uint32_t srcFrames = ch.snd->frames;

    for (int i = 0; i < outFrames; ++i) {
      double sidx = ch.posFrames;
      uint32_t idx0 = (uint32_t)sidx;
      if (idx0 >= srcFrames) { ch.finished = true; break; }

      uint32_t idx1 = std::min<uint32_t>(idx0 + 1, srcFrames - 1);
      float frac = (float)(sidx - (double)idx0);

      // stereo interleaved
      int16_t l0 = src[idx0 * 2 + 0];
      int16_t r0 = src[idx0 * 2 + 1];
      int16_t l1 = src[idx1 * 2 + 0];
      int16_t r1 = src[idx1 * 2 + 1];

      float lf = (float)l0 + ((float)l1 - (float)l0) * frac;
      float rf = (float)r0 + ((float)r1 - (float)r0) * frac;

      int outL = (int)out[i * 2 + 0] + (int)(lf * ch.volume);
      int outR = (int)out[i * 2 + 1] + (int)(rf * ch.volume);

      out[i * 2 + 0] = clampS16(outL);
      out[i * 2 + 1] = clampS16(outR);

      ch.posFrames += ch.rate;
    }
  }

  channels_.erase(std::remove_if(channels_.begin(), channels_.end(),
                                 [](const Channel& c){ return c.finished; }),
                  channels_.end());
}
