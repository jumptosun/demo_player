#ifndef _DPL_IDEMUXER_H_
#define _DPL_IDEMUXER_H_

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "decoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace dpl {

class IDemuxer
{
public:
    virtual ~IDemuxer() {};

    virtual int Open(std::string& filename) = 0;
    virtual int Close() = 0;

    virtual int StartRead() = 0;
    virtual int StopRead() = 0;
    virtual int Pause() = 0;
    virtual int Duration(AVRational* base, int64_t* dur) = 0;
    virtual int Seek(int64_t pos) = 0;

    virtual int GetStreamInfos(std::vector<AVCodecParameters*> &stream_infos) = 0;
    virtual int AddDecoder(int32_t index, std::shared_ptr<IDecoder> &dec) = 0;
    virtual void DelDecoder(int32_t index) = 0;
    virtual int FindBestStream(enum AVMediaType type, int32_t *index) = 0;
};

extern int CreateDemuxer(std::shared_ptr<IDemuxer> &d);

}

#endif
