#include <cstdio>
#include <cstdlib>
#include <thread>

#define DR_WAV_IMPLEMENTATION

#include "dr_wav.h"
#include "src/audio_render.h"
#include "src/dpl_error.h"
#include "src/dpl_log.h"

using namespace dpl;

int main(int argc, char *argv[]) {
  int ret;
  drwav wav;
  AudioRender render;
  AVCodecParameters param;
  drwav_int16 *pDecodedInterleavedPCMFrames;
  size_t numberOfSamplesActuallyDecoded;

  // sdl init
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "failed to init sdl audio\n");
    goto fail;
  }

  if (!drwav_init_file(&wav, argv[1], NULL)) {
    // Error opening WAV file.
    fprintf(stderr, "failed to open wave file\n");
    goto fail;
  }

  // TODO: fix the leak
  pDecodedInterleavedPCMFrames = (drwav_int16 *)malloc(
      wav.totalPCMFrameCount * wav.channels * sizeof(drwav_int16));
  numberOfSamplesActuallyDecoded = drwav_read_pcm_frames_s16(
      &wav, wav.totalPCMFrameCount, pDecodedInterleavedPCMFrames);


  param.sample_rate = wav.sampleRate;
  param.format = AV_SAMPLE_FMT_S16;
  param.channels = wav.channels;

  render.OpenAudio(param);
  return 0;

fail:
  drwav_uninit(&wav);
  return -1;
}
