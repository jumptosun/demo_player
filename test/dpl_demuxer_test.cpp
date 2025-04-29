#include <cstdio>
#include <string>
#include "src/dpl_error.h"
#include "src/demuxer.h"
#include "src/decoder.h"
#include "src/lockfree_queue.h"

namespace dpl {

class MockDecoder: public IDecoder
{
public:
    MockDecoder()
    {
    }

    virtual ~MockDecoder()
    {
    }

    virtual int StartDecode()
    {
        return ERROR_SUCCESS;
    }

    virtual int StopDecode()
    {
        return ERROR_SUCCESS;
    }

    virtual int Enqueue(AVPacket* pkt, std::chrono::milliseconds wait_dur)
    {
        printf("avpacket index=%d pts=%ld dts=%ld\n", pkt->stream_index, pkt->pts, pkt->dts);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        av_packet_free(&pkt);

        return ERROR_SUCCESS;
    }

    virtual void Clear()
    {
        return;
    }

    virtual int AddRender(std::shared_ptr<IRender> &r)
    {
        return ERROR_SUCCESS;
    }
};

int CreateDecoder(const AVCodecParameters &param, std::shared_ptr<IDecoder> &d)
{
    d.reset(new MockDecoder());
    return 0;
}

}

using namespace dpl;

int main(int argc, char* argv[])
{
    if(argc < 2) {
        printf("usage: %s file\n", argv[0]);
        return 0;
    }


    std::shared_ptr<IDemuxer> dmx;
    CreateDemuxer(dmx);

    std::string filename = argv[1];
    if(dmx->Open(filename) < 0) {
        printf("open failed\n");
        return -1;
    }

    std::vector<AVCodecParameters*> stream_infos;
    if(dmx->GetStreamInfos(stream_infos) < 0) {
        printf("get stream info failed\n");
        return -1;
    }

    int32_t video_index, audio_index;
    dmx->FindBestStream(AVMEDIA_TYPE_VIDEO, &video_index);
    printf("find best video stream: %d\n", video_index);

    dmx->FindBestStream(AVMEDIA_TYPE_AUDIO, &audio_index);
    printf("find best video stream: %d\n", audio_index);

    std::shared_ptr<IDecoder> dec;
    CreateDecoder((const AVCodecParameters&)*stream_infos[0], dec);

    dmx->AddDecoder(0, dec);
    dmx->StartRead();

    char action;
    int64_t pos;
    while (true) {
        scanf("%c", &action);
        if(action == 'p') {
            printf("demuxer pause\n");
            dmx->Pause();
        }

        if(action == 's') {
            printf("pos: \n");

            scanf("%ld", &pos);
            dmx->Seek(pos);
        }

        if(action == 't') {
            printf("stop\n");
            dmx->StopRead();
        }

        if(action == 'a') {
            printf("start\n");
            dmx->StartRead();
        }

        if(action == 'e') {
            break;
        }
    }

    dmx->Close();

    return 0;
}
