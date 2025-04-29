#ifndef __AUDIO_DRIVER_H__
#define __AUDIO_DRIVER_H__

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>

namespace dpl {

typedef enum AudioState {
  DPL_AD_STATE_NA = 0b00000000,
  DPL_AD_STATE_ERROR = 0b00000001,
  DPL_AD_STATE_OPENED = 0b00000010,
  DPL_AD_STATE_PLAYING = 0b00000100,
} AudioState;

inline bool IsAudioDriverError(uint8_t s) { return DPL_AD_STATE_ERROR & s; };

inline bool IsAudioDriverOpened(uint8_t s) { return DPL_AD_STATE_OPENED & s; };

inline bool IsAudioDriverPlaying(uint8_t s) {
  return DPL_AD_STATE_PLAYING & s;
};

inline bool IsAudioDriverPause(uint8_t s) {
  return IsAudioDriverOpened(s) && (!(DPL_AD_STATE_PLAYING & s));
};

// 音频格式描述（值语义）
typedef struct AudioConfig {
  int32_t freq_;
  uint16_t format_;
  uint8_t channels_;
  uint8_t silence_;
  uint16_t samples_;
  uint16_t padding_;
  uint32_t size_;
} AudioConfig;

// 抽象基类（接口定义）
class IAudioDriver {
public:
  virtual ~IAudioDriver() = default;

  // 核心操作
  virtual int32_t Play(const void *data, size_t bytes) = 0;
  virtual void Pause() = 0;
  virtual void Resume() = 0;
  virtual void Stop() = 0;

  // 状态查询
  virtual uint8_t GetState() const = 0;
  virtual double GetLatency() const = 0;
  virtual size_t GetAvailabeSpace() const = 0;

  // 动态设置
  virtual int32_t Reconfigure(const AudioConfig &format) = 0;
  // volume 范围 0~100
  virtual void SetVolume(uint32_t volume) = 0;
};

extern int CreateAudioDriver(std::unique_ptr<IAudioDriver> &ad,
                             const AudioConfig &config);

} // namespace dpl

#endif // __AUDIO_DRIVER_H__
