#include <chrono>
#include <atomic>
#include <mutex>
#include <iostream>

#include "clock_reference.h"
#include "dpl_thread.h"
#include "dpl_error.h"

namespace dpl {

class ClockReference: public IClockReference, public DplThread
{
public:
    ClockReference(): first_pts_(0), duration_(0), paused_(false) {};
    virtual ~ClockReference() {};

    virtual int SetFirstPts(std::chrono::milliseconds first_pts);

    virtual int StartClock();
    virtual int StopClock();
    virtual int TogglePause();

    virtual std::chrono::milliseconds GetTime();
    virtual std::chrono::milliseconds GetPts();
    virtual void SeekTime(std::chrono::milliseconds now);
    virtual void SeekPts(std::chrono::milliseconds now);

protected:
    virtual int Run();

private:
    std::chrono::steady_clock::time_point last_time_point_;
    std::chrono::milliseconds first_pts_;
    std::atomic<int64_t> duration_;
    std::atomic<bool> paused_;
};

static ClockReference* clock_singleton = NULL;
static std::mutex clock_lock;

int GetClockReference(IClockReference **c)
{
    if(!c) return ERROR_INVALID_ARGUMENT;

    std::lock_guard<std::mutex> guard(clock_lock);

    if(!clock_singleton)
        clock_singleton = new ClockReference();

    *c = clock_singleton;

    return ERROR_SUCCESS;
}

int ClockReference::SetFirstPts(std::chrono::milliseconds first_pts)
{
    first_pts_ = first_pts;

    return ERROR_SUCCESS;
}

int ClockReference::StartClock()
{
    last_time_point_ = std::chrono::steady_clock::now();
    return DplThread::Start();
}

int ClockReference::StopClock()
{
    DplThread::Stop();
    first_pts_ = std::chrono::milliseconds(0);
    duration_ = 0;
    paused_ = false;

    return ERROR_SUCCESS;
}

int ClockReference::TogglePause()
{
    if(paused_)
        last_time_point_ = std::chrono::steady_clock::now();

    paused_ = !paused_;

    return ERROR_SUCCESS;
}

std::chrono::milliseconds ClockReference::GetTime()
{
    return std::chrono::milliseconds(duration_);
}

std::chrono::milliseconds ClockReference::GetPts()
{
    return first_pts_ + std::chrono::milliseconds(duration_);
}

void ClockReference::SeekTime(std::chrono::milliseconds now)
{
    duration_ = now.count();
}

void ClockReference::SeekPts(std::chrono::milliseconds now)
{
    duration_ = (now - first_pts_).count();
}

int ClockReference::Run()
{
    std::chrono::steady_clock::time_point current;

    while(running_) {

        if(paused_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        current = std::chrono::steady_clock::now();
        duration_ += std::chrono::duration_cast<std::chrono::milliseconds>(current - last_time_point_).count();
        last_time_point_ = current;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return ERROR_SUCCESS;
}


}
