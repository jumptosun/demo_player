#ifndef __DPL_THREADSAFE_RINGBUFFER_H__ 
#define __DPL_THREADSAFE_RINGBUFFER_H__ 

#include <cstdint>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>
#include <cstring>
#include <assert.h>

namespace dpl {

constexpr int LOCKFREE_QUEUE_DEFAULT_SIZE = 200;

template<typename T>
class threadsafe_ringbuffer
{
public:
	using value_type = T;

private:
    value_type* queue_;
    size_t front_;
    size_t rear_;
    size_t capable_; 

    std::mutex mtx_;

public:
    threadsafe_ringbuffer(): front_(0), rear_(0), capable_(LOCKFREE_QUEUE_DEFAULT_SIZE),
            queue_(new value_type[LOCKFREE_QUEUE_DEFAULT_SIZE + 1])
    {
    }

    threadsafe_ringbuffer(uint32_t max): front_(0), rear_(0), capable_(max),
            queue_(new value_type[max + 1])
    {
    }

    ~threadsafe_ringbuffer() 
    {
        delete []queue_;
    }

    bool empty() 
    {
		std::lock_guard<std::mutex> lk(mtx_);
        return front_ == rear_;
    }


    bool resize(size_t capable) 
    {
		std::lock_guard<std::mutex> lk(mtx_);
        assert(front_ != rear_);
        
        capable_ = capable;

        delete []queue_;
        queue_ = new value_type[capable + 1];
        front_ = 0;
        rear_ = 0;

        return true;
    }

    size_t capable()
    {
		std::lock_guard<std::mutex> lk(mtx_);
        return capable_;
    }

    size_t readable()
    {
		std::lock_guard<std::mutex> lk(mtx_);
        return (rear_ - front_ + capable_ + 1) % (capable_ + 1);
    }

    size_t writeable()
    {
		std::lock_guard<std::mutex> lk(mtx_);
        return ( front_ - rear_ + capable_) % (capable_ + 1);
    }

    size_t push(const value_type *data, size_t len)
    {
		std::lock_guard<std::mutex> lk(mtx_);

        size_t writeable =  (front_ - rear_ + capable_) % (capable_ + 1);
        size_t nwrite = len < writeable ? len : writeable;

        if(nwrite <= 0) {
            return 0;
        }

        if((capable_ - rear_ + 1) >= nwrite) {
            memcpy(queue_ + rear_, data, nwrite * sizeof(value_type));

        }  else {
            int part1 =  capable_ - rear_ + 1;
            int part2 =  nwrite - part1;
            memcpy(queue_ + rear_, data, part1 * sizeof(value_type));
            memcpy(queue_, data, part2 * sizeof(value_type));
        }

        rear_ += nwrite;
        rear_ = rear_ % (capable_ + 1);

        return nwrite;
    }


    size_t pop(value_type *data, size_t len) 
    {
		std::lock_guard<std::mutex> lk(mtx_);

        size_t readable = (rear_ - front_ + capable_ + 1) % (capable_ + 1);
        size_t nread = len < readable ? len : readable;

        if(nread <= 0) 
            return 0;

        if((capable_ - front_ + 1) >= nread) {
            memcpy(data, queue_ + front_, sizeof(value_type) * nread);

        } else {
            int part1 =  capable_ - front_ + 1;
            int part2 =  nread - part1;
            memcpy(data, queue_ + front_, sizeof(value_type) * part1);
            memcpy(data, queue_, sizeof(value_type) * part2);
        }

        front_ += nread;
        front_ = front_ % (capable_ + 1);

        return nread;
    } 

};

} // dpl

#endif // __DPL_THREADSAFE_RINGBUFFER_H__ 
