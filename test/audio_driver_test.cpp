#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <chrono>
#include <iostream>
#include <libavformat/avformat.h>
#include <thread>
#include <vector>

#include "../src/audio_driver.h"
#include "../src/dpl_error.h"

namespace dpl {

// WAV 文件解码包装类
class WavLoader {
public:
  SDL_AudioSpec spec;
  Uint8 *buffer;
  Uint32 length;
  Uint32 pos;

  WavLoader(const char *path) : buffer(nullptr), length(0), pos(0) {
    if (SDL_LoadWAV(path, &spec, &buffer, &length) == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
  }

  ~WavLoader() {
    if (buffer)
      SDL_FreeWAV(buffer);
  }

  bool readChunk(void *dst, size_t bytes) {
    if (pos >= length)
      return false;
    bytes = std::min<size_t>(bytes, length - pos);
    memcpy(dst, buffer + pos, bytes);
    pos += bytes;
    return true;
  }
};

} // namespace dpl

int main(int argc, char *argv[]) {
  // 初始化 SDL
  if (SDL_Init(SDL_INIT_AUDIO) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  // 检查命令行参数
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <audio_file.wav>" << std::endl;
    SDL_Quit();
    return 1;
  }

  try {
    // 加载 WAV 文件
    dpl::WavLoader wav(argv[1]);

    // 配置音频参数
    dpl::AudioConfig config;
    config.freq_ = wav.spec.freq;
    config.channels_ = wav.spec.channels;
    config.samples_ = 4096; // 适当调整缓冲区大小

    // 转换音频格式
    switch (wav.spec.format) {
    case AUDIO_S16LSB:
      config.format_ = AV_SAMPLE_FMT_S16;
      break;
    case AUDIO_F32LSB:
      config.format_ = AV_SAMPLE_FMT_FLT;
      break;
    default:
      throw std::runtime_error("Unsupported audio format");
    }

    // 创建音频驱动
    std::unique_ptr<dpl::IAudioDriver> driver;
    if (dpl::CreateAudioDriver(driver, config) != dpl::ERROR_SUCCESS) {
      throw std::runtime_error("Failed to create audio driver");
    }

    // 开始播放
    driver->Resume();

    // 计算每秒数据量（1秒的音频数据）
    const size_t bytes_per_second = wav.spec.freq * wav.spec.channels *
                                    SDL_AUDIO_BITSIZE(wav.spec.format) / 8;

    // 分块发送音频数据
    int ret;
    std::vector<uint8_t> chunk(bytes_per_second);
    while (wav.readChunk(chunk.data(), chunk.size())) {
      ret = driver->Play(chunk.data(), chunk.size());
      if (ret != dpl::ERROR_SUCCESS) {
        std::cerr << "Playback error: " << ret << std::endl;
        break;
      }

      // 简单节流（根据实际情况调整）
      std::this_thread::sleep_for(std::chrono::milliseconds(900));
    }

    // 等待播放结束
    while (driver->GetLatency()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    SDL_Quit();
    return 1;
  }

  // 清理资源
  SDL_Quit();
  return 0;
}
