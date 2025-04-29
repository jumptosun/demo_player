#include "demuxer.h"

#include <map>
#include <mutex>
#include "dpl_log.h"
#include "dpl_thread.h"
#include "memory_free.h"
#include "dpl_error.h"

extern "C" {
#include <libavutil/avutil.h>
}

namespace dpl {

class Demuxer: public IDemuxer, public DplThread
{
public:
    Demuxer();
    virtual ~Demuxer();

    virtual int Open(std::string& filename);
    virtual int Close();

    virtual int StartRead();
    virtual int StopRead();
    virtual int Pause();

    virtual int Duration(AVRational* base, int64_t* dur);
    virtual int Seek(int64_t pos);

    virtual int GetStreamInfos(std::vector<AVCodecParameters*> &stream_infos);
    virtual int AddDecoder(int32_t index, std::shared_ptr<IDecoder> &dec);
    virtual void DelDecoder(int32_t index);
    virtual int FindBestStream(enum AVMediaType type, int32_t *index);

protected:
    virtual int Run();
    int ReadPacket();
    int SeekInternel();

private:
    std::string filename_;

    AVFormatContext *format_ctx_;

    using shared_decoder = std::shared_ptr<IDecoder>;
    std::map<int, shared_decoder> decoders_;

    AVRational timebase_;
    int64_t duration_;
    int timebase_stream_index_;

    std::mutex state_lock_;
    std::atomic<bool> opened_;

    std::atomic<bool> paused_;
    std::atomic<int64_t> seek_pos_;

    AVPacket* last_read_packet_;
};

Demuxer::Demuxer()
    : format_ctx_(0)
    , timebase_({ 0, 0})
    , duration_(0)
    , timebase_stream_index_(-1)
    , opened_(false)
    , paused_(false)
    , seek_pos_(-1)
    , last_read_packet_(NULL)
{
}

Demuxer::~Demuxer()
{
    Close();
}

int Demuxer::Open(std::string &filename)
{
    std::lock_guard<std::mutex> guard(state_lock_);

    if(opened_) {
        dpl_trace("avformat already opened file=%s, close first", filename_.c_str());
        return ERROR_SUCCESS;
    }

    if(filename.empty()) {
        dpl_error("avformat input invalid name");
        return ERROR_INVALID_ARGUMENT;
    }

    filename_ = filename;

    if(avformat_open_input(&format_ctx_, filename.c_str(), NULL, NULL) < 0) {
        dpl_error("avformat open file=%s failed", filename.c_str());
        return ERROR_LIBAV_ERROR;
	}

    if(avformat_find_stream_info(format_ctx_, NULL) < 0) {
        dpl_error("find file=%s stream failed", filename.c_str());

        avformat_close_input(&format_ctx_);
        return ERROR_LIBAV_ERROR;
    }

    av_dump_format(format_ctx_, 0, filename.c_str(), 0);

    opened_ = true;

    return ERROR_SUCCESS;
}

int Demuxer::Close()
{
    std::lock_guard<std::mutex> guard(state_lock_);

    if(!opened_)
        return ERROR_SUCCESS;

    StopRead();

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    avformat_close_input(&format_ctx_);
    avformat_free_context(format_ctx_);

    for(auto pair : decoders_) {
        auto d = pair.second;

        d->StopDecode();
    }
    decoders_.clear();

    filename_.clear();

    timebase_ = {0, 0};
    duration_ = 0;

    timebase_stream_index_ = -1;

    opened_ = false;
    seek_pos_ = -1;

    if(last_read_packet_) {
        av_packet_free(&last_read_packet_);
    }

    return ERROR_SUCCESS;
}

int Demuxer::StartRead()
{
    std::lock_guard<std::mutex> guard(state_lock_);

    if(!opened_) {
        dpl_error("demuxer wasn't opened");
        return ERROR_LOGIC_ERROR;
    }

    if(decoders_.empty()) {
        dpl_error("decoders wasn't setup");
        return ERROR_LOGIC_ERROR;
    }

    paused_ = false;

    return DplThread::Start();
}

int Demuxer::StopRead()
{
    paused_ = false;

    DplThread::Stop();
    return 0;
}

int Demuxer::Pause()
{
    paused_  = !paused_;
    return paused_;
}

int Demuxer::Duration(AVRational *base, int64_t *dur)
{
    std::lock_guard<std::mutex> guard(state_lock_);

    if(!opened_) {
        dpl_error("demuxer wasn't opened");
        return ERROR_LOGIC_ERROR;
    }

    if(!base || !dur) {
        dpl_error("demuxer invalid arguement");
        return ERROR_INVALID_ARGUMENT;
    }

    if(timebase_stream_index_ == -1) {
        dpl_error("demuxer hasn't select decoders");
        return ERROR_LOGIC_ERROR;
    }

    *base = timebase_;
    *dur = duration_;

    return 0;
}

int Demuxer::Seek(int64_t pos)
{
    seek_pos_ = pos;
    return ERROR_SUCCESS;
}

int Demuxer::GetStreamInfos(std::vector<AVCodecParameters *> &stream_infos)
{
    std::lock_guard<std::mutex> guard(state_lock_);

    if(!opened_) {
        dpl_error("demuxer wasn't opened");
        return ERROR_LOGIC_ERROR;
    }

    for(int i = 0; i < format_ctx_->nb_streams; i++) {
        stream_infos.push_back(format_ctx_->streams[i]->codecpar);
    }

    return ERROR_SUCCESS;
}

int Demuxer::AddDecoder(int32_t index, std::shared_ptr<IDecoder> &dec) 
{
    std::lock_guard<std::mutex> guard(state_lock_);

    if(!dec) {
        dpl_error("inavlid decoder");
        return ERROR_LOGIC_ERROR;
    }

    if(!opened_) {
        dpl_error("demuxer wasn't opened");
        return ERROR_LOGIC_ERROR;
    }

    if(index >= format_ctx_->nb_streams) {
        dpl_error("stream out of range");
        return ERROR_INVALID_ARGUMENT;
    }

    if(!dec) {
        dpl_error("AddDecoder invalid arguement");
        return ERROR_INVALID_ARGUMENT;
    }

    auto itr = decoders_.find(index);
    if(itr != decoders_.end()) {
        dpl_warn("stream decoders alread open, replaced");

        decoders_[index]->StopDecode();
    }

    decoders_[index] = dec;

    if(timebase_stream_index_ == -1) {
        timebase_stream_index_ = index;
        timebase_ = format_ctx_->streams[index]->time_base;
        duration_ = format_ctx_->streams[index]->duration;
    }

    return ERROR_SUCCESS;
}

int Demuxer::FindBestStream(AVMediaType type, int32_t* index)
{
    if(!index) {
        dpl_error("find best stream invalid param");
        return ERROR_INVALID_ARGUMENT;
    }

    *index = av_find_best_stream(format_ctx_, type, -1, -1, NULL, 0);

    if(*index == AVERROR_STREAM_NOT_FOUND) {
        dpl_error("stream not found");
        return ERROR_FAIL;
    }

    return ERROR_SUCCESS;
}

void Demuxer::DelDecoder(int32_t index)
{
    std::lock_guard<std::mutex> guard(state_lock_);

    if(!opened_) {
        dpl_error("demuxer wasn't opened");
        return;
    }

    if(index >= format_ctx_->nb_streams) {
        return;
    }

    auto itr = decoders_.find(index);
    if(itr != decoders_.end()) {
        decoders_[index]->StopDecode();
    }
}

int Demuxer::Run()
{
    while(running_) {
        if(seek_pos_ != -1) {
            SeekInternel();
            seek_pos_ = -1;
        }

        if(!paused_) {
            if(ReadPacket() < 0) {
                break;
            }

        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    return ERROR_SUCCESS;
}

int Demuxer::ReadPacket()
{
    int ret;
    shared_decoder decoder;

    if(!last_read_packet_) {
        last_read_packet_ = av_packet_alloc();
        if((ret = av_read_frame(format_ctx_, last_read_packet_)) < 0) {
            dpl_error("av_read_frame error or end of file");
            return ERROR_LIBAV_ERROR;
        }
    }

    auto itr = decoders_.find(last_read_packet_->stream_index);
    if(itr != decoders_.end()) {
        decoder = itr->second;
        if(decoder->Enqueue(last_read_packet_, std::chrono::milliseconds(10)) == ERROR_SUCCESS) {
            last_read_packet_ = NULL;
        }
    } else {
        av_packet_free(&last_read_packet_);
    }

    return ERROR_SUCCESS;
}

int Demuxer::SeekInternel()
{
    int ret = av_seek_frame(format_ctx_, timebase_stream_index_, seek_pos_, AVSEEK_FLAG_BACKWARD);
    if(ret < 0) {
        dpl_error("demuxer seek failed, ret=%d", ret);
        return ERROR_LIBAV_ERROR;
    }

    for(auto pair: decoders_) {
        pair.second->Clear();
    }

    return ERROR_SUCCESS;
}

int CreateDemuxer(std::shared_ptr<IDemuxer> &d)
{
    d.reset(static_cast<IDemuxer*>(new Demuxer));

    if(!(bool)(d)) {
        return ERROR_FAIL;
    }

    return ERROR_SUCCESS;
}

}
