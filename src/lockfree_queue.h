#ifndef __DPL_LOCKFREE_QUEUE_H__ 
#define __DPL_LOCKFREE_QUEUE_H__

#include <cstdint>
#include <vector>
#include <chrono>
#include <atomic>
#include <thread>
#include <functional>

namespace dpl {

constexpr int LOCKFREE_QUEUE_DEFAULT_SIZE = 200;

template<typename T>
class lockfree_queue
{
public:
	using value_type = T;

    struct node {

        enum node_state {
            empty_state = 0,
            full_state  = 1,
        };

        value_type data_;
        std::atomic<int> sta_;

        node(): sta_(empty_state) {};
        ~node() {};
    };

private:
    std::vector<node> queue_;
    std::atomic<uint32_t> front_;
    std::atomic<uint32_t> rear_;
    std::atomic<uint32_t> max_size_; 

    std::atomic<int> size_;
    std::atomic<int> writeable_;


public:
    lockfree_queue(): front_(0), rear_(0), size_(0), 
            writeable_(LOCKFREE_QUEUE_DEFAULT_SIZE), max_size_(LOCKFREE_QUEUE_DEFAULT_SIZE),
            queue_(LOCKFREE_QUEUE_DEFAULT_SIZE)
    {
    }

    lockfree_queue(uint32_t max): front_(0), rear_(0), size_(0), writeable_(max), max_size_(max),
            queue_(max)
    {
    }

    ~lockfree_queue() 
    {
    }

    bool empty() 
    {
        return size_ == 0;
    }

    int size()
    {
        return size_;
    }

    bool push(const value_type& data)
    {
        if(writeable_.fetch_sub(1, std::memory_order_seq_cst) <= 0) {
            writeable_.fetch_add(1, std::memory_order_seq_cst);
            return false;
        }

        uint32_t pos = rear_.fetch_add(1, std::memory_order_seq_cst) % max_size_;

        int node_empty = node::empty_state;
        while(!queue_[pos].sta_.compare_exchange_strong(node_empty, node::full_state)) {
            node_empty = node::empty_state;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        queue_[pos].data_ = data;
        size_.fetch_add(1, std::memory_order_seq_cst);

        return true;
    }

    template <typename rep, typename period>
    bool time_push(const value_type& data, std::chrono::duration<rep, period> wait_dur)
    {
        auto deadline = std::chrono::steady_clock::now() + wait_dur;

        uint32_t pos = rear_.fetch_add(1, std::memory_order_seq_cst) % max_size_;

        int node_empty = node::empty_state;
        while(!queue_[pos].sta_.compare_exchange_strong(node_empty, node::full_state)) {
            if(std::chrono::steady_clock::now() >= deadline)
                return false;

            node_empty = node::empty_state;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        queue_[pos].data_ = data;
        size_.fetch_add(1, std::memory_order_seq_cst);
        writeable_.fetch_sub(1, std::memory_order_seq_cst);

        return true;
    }

    bool try_push(const value_type& data)
    {
        if(writeable_.fetch_sub(1, std::memory_order_seq_cst) <= 0) {
            writeable_.fetch_add(1, std::memory_order_seq_cst);
            return false;
        }

        uint32_t pos = rear_.fetch_add(1, std::memory_order_seq_cst) % max_size_;

        int node_empty = node::empty_state;
        if(!queue_[pos].sta_.compare_exchange_strong(node_empty, node::full_state)) {
            return false;
        }

        queue_[pos].data_ = data;
        size_.fetch_add(1, std::memory_order_seq_cst);

        return true;
    }

    bool pop(value_type &data) 
    {
        uint32_t pos;
        int node_full = node::full_state;

        while(true) {
            if(size_ == 0) return false;
            
            pos = front_ % max_size_;
            data = queue_[pos].data_;
            node_full = node::full_state;

            if(queue_[pos].sta_.compare_exchange_strong(node_full, node::empty_state)) 
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        front_.fetch_add(1, std::memory_order_seq_cst);

        size_.fetch_sub(1, std::memory_order_seq_cst);
        writeable_.fetch_add(1, std::memory_order_seq_cst);
        
        return true;
    } 

    template <typename rep, typename period>
    bool time_pop(value_type &data, std::chrono::duration<rep, period> wait_dur)
    {
        uint32_t pos;
        auto deadline = std::chrono::steady_clock::now() + wait_dur;
        int node_full = node::full_state;

        while(true) {
            pos = front_ % max_size_;
            data = queue_[pos].data_;
            node_full = node::full_state;

            if(queue_[pos].sta_.compare_exchange_strong(node_full, node::empty_state))
                break;

            if(std::chrono::steady_clock::now() >= deadline)
                return false;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        front_.fetch_add(1, std::memory_order_seq_cst);

        size_.fetch_sub(1, std::memory_order_seq_cst);
        writeable_.fetch_add(1, std::memory_order_seq_cst);

        return true;
    }

    bool try_pop(value_type &data)
    {
        uint32_t pos;
        int node_full = node::full_state;

        if(size_ == 0) return false;

        pos = front_ % max_size_;
        data = queue_[pos].data_;

        if(!queue_[pos].sta_.compare_exchange_strong(node_full, node::empty_state))
            return false;

        front_.fetch_add(1, std::memory_order_seq_cst);

        size_.fetch_sub(1, std::memory_order_seq_cst);
        writeable_.fetch_add(1, std::memory_order_seq_cst);

        return true;
    }
};

} // dpl

#endif // __DPL_LOCKFREE_QUEUE_H__
