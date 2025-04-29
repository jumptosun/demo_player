#ifndef __DPL_THREAD_H__
#define __DPL_THREAD_H__

#include <cstddef>
#include <thread>
#include <atomic>
#include "memory_free.h"

namespace dpl {
    
class DplThread
{
public:
    DplThread() 
        : thread_ptr_(nullptr)
        , running_(false) {}

    virtual ~DplThread() 
    {
    }

    int Start()
    {
        bool off = false;
        if(!running_.compare_exchange_strong(off, true, std::memory_order_seq_cst))
            return 0;

        if(OnStart() < 0) 
            return -1;

        if((thread_ptr_ = new std::thread(&DplThread::Cycle, this)) == nullptr) {
            return -1;
        }

        thread_ptr_->detach();

        return 0;
    };

    void Stop()
    {
        bool on = true;
        if(!running_.compare_exchange_strong(on, false, std::memory_order_seq_cst))
            return;

        OnStop();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        dpl_freep(thread_ptr_);
    }

    int Cycle()
    {
        if(BeforeRun() < 0) {
            running_.store(false, std::memory_order_seq_cst);
            return -1;
        }

        Run();

        AfterRun();

        return 0;
    }

protected:
    virtual int OnStart() { return 0; };
    virtual void OnStop() {};

    virtual int BeforeRun() { return 0; };
    virtual int Run() = 0;
    virtual void AfterRun() {};

    std::thread *thread_ptr_;
    std::atomic<bool> running_;
};


} // dpl 

#endif //__DPL_THREAD_H__
