#include "audio_driver.h"

#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_endian.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <libavformat/avformat.h>
#include <map>
#include <memory>
#include <mutex>
#include <sys/types.h>

#include "dpl_error.h"
#include "dpl_log.h"

// TODO: 完成线程安全，和单元测试

namespace dpl {

std::map<int, int> kAudioFormatCvtMap = {
    {AV_SAMPLE_FMT_U8, AUDIO_U8},
    {AV_SAMPLE_FMT_S16,
     (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? AUDIO_S16LSB : AUDIO_S16MSB},
    {AV_SAMPLE_FMT_S32,
     (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? AUDIO_S32LSB : AUDIO_S32MSB},
    {AV_SAMPLE_FMT_FLT, AUDIO_F32},

    // Planar formats 转 packed（假设调用方已做packed处理）
    {AV_SAMPLE_FMT_U8P, AUDIO_U8},      // Planar 转 packed unsigned 8-bit
    {AV_SAMPLE_FMT_S16P, AUDIO_S16SYS}, // Planar 转 packed signed 16-bit
    {AV_SAMPLE_FMT_S32P, AUDIO_S32SYS}, // Planar 转 packed signed 32-bit
    {AV_SAMPLE_FMT_FLTP, AUDIO_F32SYS}, // Planar 转 packed 32-bit float

    // 不支持的格式保留为0
    {AV_SAMPLE_FMT_DBLP, 0}, // Planar 转 packed 64-bit float
    {AV_SAMPLE_FMT_DBL, 0},  // 64-bit float
    {AV_SAMPLE_FMT_S64, 0},  // SDL不支持64-bit integer
    {AV_SAMPLE_FMT_S64P, 0}, // SDL不支持64-bit integer planar
    {AV_SAMPLE_FMT_NB, 0},   // 特殊占位符
    {AV_SAMPLE_FMT_NONE, 0}  // 无效格式
};

inline uint8_t SetAudioDriverError(uint8_t s) {
  return DPL_AD_STATE_ERROR | s;
};

inline uint8_t UnsetAudioDriverError(uint8_t s) {
  return (~uint8_t(DPL_AD_STATE_ERROR)) & s;
};

inline uint8_t SetAudioDriverOpened(uint8_t s) {
  return DPL_AD_STATE_OPENED | s;
};

inline uint8_t UnsetAudioDriverOpened(uint8_t s) {
  return (~uint8_t(DPL_AD_STATE_OPENED)) & s;
};

inline uint8_t SetAudioDriverPlaying(uint8_t s) {
  return DPL_AD_STATE_PLAYING | s;
};

inline uint8_t UnsetAudioDriverPlaying(uint8_t s) {
  return (~uint8_t(DPL_AD_STATE_PLAYING)) & s;
};

class SdlDriver : public IAudioDriver {
public:
  SdlDriver();
  virtual ~SdlDriver();

  int Init(const AudioConfig &config);

  int32_t Play(const void *data, size_t bytes) override;

  void Pause() override;
  void Resume() override;
  void Stop() override;

  uint8_t GetState() const override;

  double GetLatency() const override;

  size_t GetAvailabeSpace() const override;

  int32_t Reconfigure(const AudioConfig &format) override;

  void SetVolume(uint32_t volume) override;

private:
  int AdjustVolume(uint8_t *&dst, const uint8_t *src, uint32_t len);

private:
  uint32_t sdl_volume_;
  uint8_t state_;
  AudioConfig config_;
  SDL_AudioSpec sdl_spec_;
  SDL_AudioDeviceID device_id_;
  std::unique_ptr<uint8_t[]> audio_buffer_;
  size_t audio_buffer_size_;
  std::mutex mtx_;
};

SdlDriver::SdlDriver()
    : sdl_volume_(SDL_MIX_MAXVOLUME), state_(DPL_AD_STATE_NA), config_({0}),
      sdl_spec_({0}), audio_buffer_size_(0), device_id_(0) {}

SdlDriver::~SdlDriver() { Stop(); }

int SdlDriver::Init(const AudioConfig &config) {
  if (IsAudioDriverOpened(state_)) {
    Stop();
  }

  const std::lock_guard<std::mutex> lock(mtx_);

  auto fmt_it = kAudioFormatCvtMap.find(config.format_);
  if (fmt_it == kAudioFormatCvtMap.end() || fmt_it->second == 0) {
    dpl_error("Unsupported audio format: %d", config.format_);
    return ERROR_AUDIO_UNSUPPORTED_FORMAT;
  }

  // transform AudioConfig to SDL_AudioSpec
  SDL_AudioSpec desired = {0};
  desired.freq = config.freq_;
  desired.format = static_cast<SDL_AudioFormat>(fmt_it->second);
  desired.channels = static_cast<Uint8>(config.channels_);
  desired.samples = static_cast<Uint16>(config.samples_);
  desired.padding = static_cast<Uint16>(config.padding_);
  desired.size = static_cast<Uint32>(config.size_);
  desired.callback = nullptr; // Use queue-based API
  desired.userdata = nullptr;

  SDL_AudioSpec obtained;
  device_id_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained,
                                   SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if (device_id_ == 0) {
    dpl_error("Failed to open audio device: %s", SDL_GetError());
    return ERROR_AUDIO_OPEN_DEVICE_FAILED;
  }

  sdl_spec_ = obtained;
  config_ = config;

  state_ = SetAudioDriverOpened(state_);

  return ERROR_SUCCESS;
}

int32_t SdlDriver::Play(const void *data, size_t bytes) {
  if (device_id_ == 0) {
    return ERROR_AUDIO_NOT_OPEN;
  }

  const std::lock_guard<std::mutex> lock(mtx_);

  uint8_t *dst = nullptr;

  AdjustVolume(dst, (const uint8_t *)data, bytes);

  if (SDL_QueueAudio(device_id_, dst, bytes) != 0) {
    dpl_error("SDL_QueueAudio failed: %s", SDL_GetError());
    state_ = SetAudioDriverError(state_);
    return ERROR_AUDIO_PLAY_FAILED;
  }

  return ERROR_SUCCESS;
}

int SdlDriver::AdjustVolume(uint8_t *&dst, const uint8_t *src, uint32_t len) {
  if (sdl_volume_ == SDL_MIX_MAXVOLUME) {
    dst = (uint8_t *)src;
    return ERROR_SUCCESS;
  }

  if (audio_buffer_size_ < len) {
    audio_buffer_.reset(new uint8_t[len]);
    audio_buffer_size_ = len;
  }

  dst = audio_buffer_.get();
  SDL_MixAudioFormat(dst, src, sdl_spec_.format, len, sdl_volume_);
  return ERROR_SUCCESS;
}

void SdlDriver::Pause() {
  const std::lock_guard<std::mutex> lock(mtx_);

  if (device_id_ != 0) {
    SDL_PauseAudioDevice(device_id_, 1);
    state_ = UnsetAudioDriverPlaying(state_);
  }
}

void SdlDriver::Resume() {
  const std::lock_guard<std::mutex> lock(mtx_);

  if (device_id_ != 0) {
    SDL_PauseAudioDevice(device_id_, 0);
    state_ = SetAudioDriverPlaying(state_);
  }
}

void SdlDriver::Stop() {
  const std::lock_guard<std::mutex> lock(mtx_);

  if (device_id_ != 0) {
    SDL_ClearQueuedAudio(device_id_);
    SDL_CloseAudioDevice(device_id_);

    device_id_ = 0;
    state_ = DPL_AD_STATE_NA;
    config_ = {0};
    sdl_spec_ = {0};
  }
}

uint8_t SdlDriver::GetState() const { return state_; }

double SdlDriver::GetLatency() const {
  if (device_id_ == 0)
    return 0.0;

  const Uint32 queued = SDL_GetQueuedAudioSize(device_id_);
  const int bytes_per_sec = sdl_spec_.freq * sdl_spec_.channels *
                            SDL_AUDIO_BITSIZE(sdl_spec_.format) / 8;
  return bytes_per_sec > 0 ? static_cast<double>(queued) / bytes_per_sec : 0.0;
}

size_t SdlDriver::GetAvailabeSpace() const { return 0; }

int32_t SdlDriver::Reconfigure(const AudioConfig &new_config) {
  return Init(new_config);
}

void SdlDriver::SetVolume(uint32_t volume) {
  const std::lock_guard<std::mutex> lock(mtx_);

  sdl_volume_ = volume * 128 / 100;
}

int CreateAudioDriver(std::unique_ptr<IAudioDriver> &ad,
                      const AudioConfig &config) {
  auto driver = std::make_unique<SdlDriver>();
  const int ret = driver->Init(config);
  if (ret == ERROR_SUCCESS) {
    ad = std::move(driver);
  }
  return ret;
}

} // namespace dpl
