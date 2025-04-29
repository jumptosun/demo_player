
#include <cstdio>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <functional>

#include "src/lockfree_queue.h"
#include "src/dpl_thread.h"
#include "src/decoder.h"
#include "src/demuxer.h"
#include "src/dpl_error.h"

using namespace dpl;

class MockRender: public IRender, public DplThread
{
public:
    MockRender()
        : count_(0)
    {
    }

    ~MockRender()
    {   
        Clear();
    }

    virtual int StartRender()
    {
        Start();
        return 0;
    }

    virtual int StopRender()
    {
        Stop();

        this->Clear();
        return 0;
    }


    virtual int TogglePause()
    {
        return 0;
    }

    virtual int Enqueue(AVFrame* frame, std::chrono::milliseconds wait_dur)
    {
        if(av_frames_.time_push(frame, wait_dur)) {
            return ERROR_SUCCESS;
        }

        return ERROR_FAIL;
    }

    virtual int Clear()
    {
        AVFrame* frame;
        while(!av_frames_.empty()) {
            if(av_frames_.try_pop(frame)) {
                av_frame_free(&frame);
            }
        }

        return 0;
    }

    virtual int Run()
    {
        AVFrame* frame;
        char filename[64];
        FILE* pic;
        int componet_size = 0;

        while(running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            if(!av_frames_.try_pop(frame)) {
                printf("pop frame failed\n");
                continue;
            }

            snprintf(filename, 64, "decoder_test_pic_%03d", count_++);

            if((pic = fopen(filename, "w+")) == NULL) {
                fprintf(stderr, "open picture file failed");

                av_frame_free(&frame);
                continue;
            }

            componet_size = 0;
            for(auto i = 0; i < 3; i++) {
                componet_size = frame->height * frame->linesize[i];
                fwrite(frame->data[i], 1, componet_size, pic);
            }

            printf("frame pts: %ld\n", frame->pts);

            av_frame_free(&frame);
            fclose(pic);

            Clear();
        }

        return 0;
    }

private:
    int count_;
    lockfree_queue<AVFrame*> av_frames_;
};

int main(int argc, char* argv[])
{
    std::shared_ptr<IDemuxer> dm;
    std::shared_ptr<IDecoder> dc;
    std::shared_ptr<IRender> r(new MockRender);

    if(argc < 2) {
        fprintf(stderr,
                "missing input file.\n"
                "usage:\n"
                "    %s input file\n", argv[0]);

        return 0;
    }

    std::string input = argv[1];

    if(CreateDemuxer(dm) < 0) {
        fprintf(stderr, "create demuxer failed\n");
        return -1;
    }

    if(dm->Open(input) < 0) {
        fprintf(stderr, "demuxer open input failed\n");
        return -1;
    }

    std::vector<AVCodecParameters*> input_stream_param;
    if(dm->GetStreamInfos(input_stream_param) < 0) {
        fprintf(stderr, "demuxer open input failed\n");
        return -1;
    }

    int video_index;
    if(ERROR_SUCCESS != dm->FindBestStream(AVMEDIA_TYPE_VIDEO, &video_index)) {
        fprintf(stderr, "demuxer can't find video stream\n");
        return -1;
    }

    if(CreateDecoder(*input_stream_param[video_index], dc) < 0) {
        fprintf(stderr, "create decoder failed\n");
        return -1;
    }

    dc->AddRender(r);
    dm->AddDecoder(video_index, dc);

    dm->StartRead();
    dc->StartDecode();
    r->StartRender();

    char action;
    while(true) {
        scanf("%c", &action);
        if(action == 't') {
            printf("decode stop\n");
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(100));
    }

    dm->StopRead();
    dc->StopDecode();
    r->StopRender();

    return 0;
}
