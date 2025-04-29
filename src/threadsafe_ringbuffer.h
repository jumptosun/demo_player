#ifndef __DPL_THREADSAFE_RINGBUFFER_H__ 
#define __DPL_THREADSAFE_RINGBUFFER_H__ 

#include <cstdint>
#include <vector>
#include <chrono>
#include <mutex>
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

    size_t readable_;
    size_t writeable_;

    std::mutex mtx_;

public:
    threadsafe_ringbuffer(): front_(0), rear_(0), readable_(0), 
            writeable_(LOCKFREE_QUEUE_DEFAULT_SIZE), capable_(LOCKFREE_QUEUE_DEFAULT_SIZE),
            queue_(new value_type[LOCKFREE_QUEUE_DEFAULT_SIZE])
    {
    }

    threadsafe_ringbuffer(uint32_t max): front_(0), rear_(0), readable_(0), writeable_(max), capable_(max),
            queue_(new value_type[max])
    {
    }

    ~threadsafe_ringbuffer() 
    {
        delete []queue_;
    }

    bool empty() 
    {
		std::lock_guard<std::mutex> lk(mtx_);
        return readable_ == 0;
    }

    void clear()
    {
		std::lock_guard<std::mutex> lk(mtx_);
        front_ = 0;
        rear_ = 0;
    }


    bool resize(size_t capable) 
    {
		std::lock_guard<std::mutex> lk(mtx_);
        
        if(queue_ != nullptr) {
            delete []queue_;

        }
        
        capable_ = capable;

        queue_ = new value_type[capable_];
        front_ = 0;
        rear_ = 0;
        writeable_ = capable_;
        readable_ = capable;

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
        return readable_;
    }

    size_t writeable()
    {
		std::lock_guard<std::mutex> lk(mtx_);
        return writeable_;
    }

    size_t push(const value_type *data, size_t len)
    {
		std::lock_guard<std::mutex> lk(mtx_);

        size_t nwrite = len < writeable_ ? len : writeable_;

        if(nwrite <= 0) {
            return 0;
        }

        writeable_ -= nwrite;

        if((capable_ - rear_) >= nwrite ) {
            memcpy(queue_ + rear_, data, nwrite * sizeof(value_type));

        }  else {
            int part1 =  capable_ - rear_;
            int part2 =  nwrite - part1;
            memcpy(queue_ + rear_, data, part1 * sizeof(value_type));
            memcpy(queue_, data, part2 * sizeof(value_type));
        }

        rear_ += nwrite;
        rear_ = rear_ % capable_;

        readable_ += nwrite;

        return nwrite;
    }


    size_t pop(value_type *data, size_t len) 
    {
		std::lock_guard<std::mutex> lk(mtx_);

        size_t nread = len < readable_ ? len : readable_;

        if(nread <= 0) 
            return 0;

        readable_ -= nread;

        if((capable_ - front_) >= nread) {
            memcpy(data, queue_ + front_, sizeof(value_type) * nread);

        } else {
            int part1 =  capable_ - front_;
            int part2 =  nread - part1;
            memcpy(data, queue_ + front_, sizeof(value_type) * part1);
            memcpy(data, queue_, sizeof(value_type) * part2);
        }

        front_ += nread;
        front_ = front_ % capable_;

        writeable_ += nread;

        return nread;
    } 

};

} // dpl

#endif // __DPL_THREADSAFE_RINGBUFFER_H__ 
