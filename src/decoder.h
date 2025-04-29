#ifndef _DPL_DECODER_H_
#define _DPL_DECODER_H_

#include <chrono>
#include <memory>

#include "render.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace dpl {

class IDecoder
{
public:
    virtual ~IDecoder() {};

    virtual int StartDecode() = 0;
    virtual int StopDecode() = 0;

    virtual int Enqueue(AVPacket* pkt, std::chrono::milliseconds wait_dur) = 0;

    virtual void Clear() = 0;

    virtual int AddRender(std::shared_ptr<IRender> &r) = 0;
};

extern int CreateDecoder(const AVCodecParameters &param, std::shared_ptr<IDecoder> &d);

}

#endif
