
#include <stdio.h>

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_log.h>

void MyAudioCallback(void* userdata, Uint8* stream, int len)
{
    printf("stream length=%d\n", len);
}

int main(int argc, char* argv[])
{
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;

    SDL_Init(SDL_INIT_AUDIO);

    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 4096;
    want.callback = MyAudioCallback; /* you wrote this function elsewhere -- see SDL_AudioSpec for details */

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    } else {
        if (have.format != want.format) /* we let this one thing change. */
            SDL_Log("We didn't get Float32 audio format.");
    }
    SDL_PauseAudioDevice(dev, 0); /* start audio playing. */
    SDL_Delay(5000); /* let the audio callback play some sound for 5 seconds. */
    SDL_CloseAudioDevice(dev);

    SDL_Quit();
}
