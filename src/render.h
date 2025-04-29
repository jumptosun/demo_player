#ifndef _AUDIO_RENDER_H_
#define _AUDIO_RENDER_H_

#include <memory>
#include <cstdint>
#include <chrono>

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
}

namespace dpl {

enum DPL_RENDER_TYPE
{
    RENDER_TYPE_VIDEO = 0,
    RENDER_TYPE_AUDIO,
    RENDER_TYPE_UNKNOWN,
};

class IRender
{
public:
    virtual ~IRender() {}

    virtual int Enqueue(AVFrame* frame, std::chrono::milliseconds wait_dur) = 0;
    virtual int Clear() = 0;

    /**
     * @brief StartRender
     * 	start render the data
     * @return
     */
    virtual int StartRender() = 0;
    virtual int StopRender() = 0;
    virtual int TogglePause() = 0;
};

extern int CreateRender(int type, std::shared_ptr<IRender> &r);

};

#endif // _AUDIO_RENDER_H_
