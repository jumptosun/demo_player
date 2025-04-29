#ifndef _DPL_CONTROLLER_H_
#define _DPL_CONTROLLER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "render.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace dpl {

typedef enum DplWhence
{
    DPL_SEEK_NA = 0,
    DPL_SEEK_SET,
    DPL_SEEK_END,
    DPL_SEEK_CUR,
} DplWhence;

typedef enum DplPlayerStatus
{
    DPL_PLAYER_STATUS_NA = 0,
    DPL_PLAYER_STATUS_OPENED = 0b0001,
    DPL_PLAYER_STATUS_PLAYING = 0b0010,
} DplPlayerStatus;

class IController
{
public:
    virtual ~IController() = default;

    virtual int Open(std::string &filename) = 0;
    virtual int Close() = 0;

    virtual int Play() = 0;
    virtual int TogglePause() = 0;

    virtual int GetStatus() = 0;
    virtual int IsOpened() = 0;
    virtual int IsPlaying() = 0;


    virtual int GetStreamInfos(std::vector<AVCodecParameters*> &stream_infos) const = 0;
    virtual int SelectAudioTrack(int index) = 0;
    virtual int SelectSubtitle(int index) = 0;

    virtual int Duration() const = 0 ;
    virtual int Seek(long offset, int whence) = 0;

    virtual int SetConfig(const std::string& key, const std::string& value) = 0;
};

extern int GetController(std::unique_ptr<IController>& c);

}

#endif
