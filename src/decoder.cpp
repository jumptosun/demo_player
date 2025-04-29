
#include <atomic>

#include "decoder.h"
#include "dpl_thread.h"
#include "dpl_log.h"
#include "dpl_error.h"
#include "lockfree_queue.h"

namespace dpl {


class Decoder: public IDecoder, public DplThread
{
    const int DPL_DECODER_CACHE_LENGTH = 200;
public:
    Decoder();
    virtual ~Decoder();

    int Open(const AVCodecParameters& param);

    virtual int StartDecode();
    virtual int StopDecode();

    virtual int Enqueue(AVPacket* pkt, std::chrono::milliseconds wait_dur);
    virtual void Clear();

    virtual int AddRender(std::shared_ptr<IRender> &r);

protected:
    virtual int Run();

private:
    int DecodeFrame();
    int Close();
    int ClearPkts();

private:
    lockfree_queue<AVPacket*> av_pkts_;
    AVCodecContext* codec_cxt_;
    std::shared_ptr<IRender> render_;
    std::atomic<bool>  to_clear_;
    AVFrame *last_frame_;
};


Decoder::Decoder()
    : codec_cxt_(NULL)
    , render_(NULL)
    , to_clear_(false)
    , last_frame_(NULL)
    , av_pkts_(DPL_DECODER_CACHE_LENGTH)
{
}

Decoder::~Decoder()
{
    Close();
}

int Decoder::Open(const AVCodecParameters &param)
{
    auto codec = avcodec_find_decoder(param.codec_id);
    if(!codec) {
        dpl_error("cant't find codec");
        return ERROR_INVALID_ARGUMENT;
    }

    codec_cxt_ = avcodec_alloc_context3(codec);
    if(!codec_cxt_) {
        dpl_error("cant't alloc codec context");
        return ERROR_LIBAV_ERROR;
    }

    if(avcodec_parameters_to_context(codec_cxt_, &param) < 0) {
        dpl_error("assign parameters failed");
        return ERROR_INVALID_ARGUMENT;
    }

    if(avcodec_open2(codec_cxt_, codec, NULL) < 0) {
        dpl_error("cant't open codec context");
        return ERROR_LIBAV_ERROR;
    }

    return  ERROR_SUCCESS;
}

int Decoder::StartDecode()
{
    if(!render_) {
        dpl_error("frame handler wasn't setup");
        return ERROR_LOGIC_ERROR;
    }

    to_clear_ = false;

    return Start();
}

int Decoder::StopDecode()
{
    Stop();
    return ERROR_SUCCESS;
}

int Decoder::Enqueue(AVPacket *pkt, std::chrono::milliseconds wait_dur)
{
    if(av_pkts_.time_push(pkt, wait_dur)) {
        return ERROR_SUCCESS;
    }

    return ERROR_FAIL;
}

void Decoder::Clear()
{
    to_clear_.store(true, std::memory_order_seq_cst);
}

int Decoder::AddRender(std::shared_ptr<IRender> &r)
{
    if(!r) {
        dpl_error("invalidd render");
        return ERROR_INVALID_ARGUMENT;
    }


    render_ = r;
    return ERROR_SUCCESS;
}

int Decoder::Run()
{
    while(running_) {
        if(to_clear_.load(std::memory_order_seq_cst)) {
            ClearPkts();
            av_frame_free(&last_frame_);
            to_clear_ = false;
        }

        if(DecodeFrame() != ERROR_SUCCESS) {
            break;
        }
    }

    return 0;
}

int Decoder::DecodeFrame()
{
    AVPacket* pkt;
    char av_str[256];
    int ret;

    if(!last_frame_) {
        if(!av_pkts_.time_pop(pkt, std::chrono::milliseconds(10))) {
            return 0;
        }


        if((ret = avcodec_send_packet(codec_cxt_, pkt)) < 0) {
            if(ret == AVERROR(EAGAIN)) {
                return ERROR_SUCCESS;
            }

            av_strerror(ret, av_str, 256);
            dpl_error("avcodec_send_packet failed, %s", av_str);
            return ERROR_LIBAV_ERROR;
        }

        last_frame_ = av_frame_alloc();
        if((ret = avcodec_receive_frame(codec_cxt_, last_frame_)) < 0) {
            if(ret == AVERROR(EAGAIN)) {
                return ERROR_SUCCESS;
            }

            av_strerror(ret, av_str, 256);
            dpl_error("avcodec_receive_frame failed, %s", av_str);
            av_frame_free(&last_frame_);
            return ERROR_LIBAV_ERROR;
        }
    }

    if(render_->Enqueue(last_frame_, std::chrono::milliseconds(40)) == ERROR_SUCCESS) {
        last_frame_ = NULL;
    }

    return ERROR_SUCCESS;
}

int Decoder::Close()
{
    this->Stop();

    ClearPkts();

    if(last_frame_) {
        av_frame_free(&last_frame_);
    }

    if(codec_cxt_) {
        avcodec_close(codec_cxt_);
        avcodec_free_context(&codec_cxt_);
    }

    return 0;
}

int Decoder::ClearPkts()
{
    AVPacket* pkt;
    while(!av_pkts_.empty()) {
        if(av_pkts_.try_pop(pkt))
            av_packet_free(&pkt);
    }
    
    return ERROR_SUCCESS;
}

int CreateDecoder(const AVCodecParameters &param, std::shared_ptr<IDecoder> &d)
{
    d.reset(static_cast<IDecoder*>(new Decoder));

    if(((Decoder*)d.get())->Open(param) != ERROR_SUCCESS) {
        return ERROR_FAIL;
    }

    return ERROR_SUCCESS;
}

}