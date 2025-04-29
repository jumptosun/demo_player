
#define CATCH_CONFIG_MAIN

#define __STDC_CONSTANT_MACROS

#include <cstdlib>
#include <thread>

#include "catch.hpp"
#include "src/dpl_log.h"
#include "src/dpl_error.h"
#include "src/audio_render.h"

using namespace dpl;

TEST_CASE("Test audio render", "[AlsaRender]" ) {
    int ret;
    IAudioRender* ar;

    ret = CreateAudioRender(&ar);
    REQUIRE(ret == ERROR_SUCCESS);

    SECTION("init device", "[InitDevice]") {
        ret = ar->InitDevice(AV_SAMPLE_FMT_S16, 2, 9600, std::move(AVRational({1, 1000})));
        REQUIRE(ret == ERROR_SUCCESS);
    }

    SECTION("enqueue 200 empty frame", "[Enqueue]") {
        AVFrame* f;

        for(auto i = 0; i < 0; i++) {
            f = av_frame_alloc();
            REQUIRE(f != nullptr);

            f->format = AV_SAMPLE_FMT_S16;
            f->nb_samples = 2400;
            f->channel_layout = AV_CH_LAYOUT_STEREO;

            ret = av_frame_get_buffer(f, 0);
            REQUIRE(ret == 0);

            ret = ar->Enqueue(f, std::chrono::milliseconds(2));
            REQUIRE(ret == ERROR_SUCCESS);
        }

        ret = ar->Enqueue(nullptr, std::chrono::milliseconds(2));
        REQUIRE(ret != ERROR_SUCCESS);

    }

    SECTION("start render", "[StartRender]") {

        ret = ar->StartRender();
        REQUIRE(ret == ERROR_SUCCESS);

    }
    
    std::this_thread::sleep_for(std::chrono::seconds(10));

    SECTION("toggle pause", "[TogglePause]") {

        ret = ar->TogglePause();
        REQUIRE(ret == ERROR_SUCCESS);

    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    SECTION("toggle pause", "[TogglePause]") {

        ret = ar->TogglePause();
        REQUIRE(ret == ERROR_SUCCESS);
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    SECTION("Stop render", "[StopRender]") {
        ret = ar->Clear();

        ret = ar->StopRender();
        REQUIRE(ret == ERROR_SUCCESS);
    }
}
