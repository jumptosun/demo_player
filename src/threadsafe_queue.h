#ifndef __THREADSAFE_QUEUE_H__
#define __THREADSAFE_QUEUE_H__

#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

#include <stdlib.h>
#include <time.h>

template<typename T>
class threadsafe_queue {
private:
	mutable std::mutex mtx;
	mutable std::condition_variable data_cond;
	using queue_type = std::queue<T>;
	queue_type data_queue;

public:
	using value_type = typename queue_type::value_type;
	using container_type = typename queue_type::container_type;
	
	// 默认构造函数
	threadsafe_queue() = default;
	
	// 使用容器为参数的构造函数
	explicit threadsafe_queue(const container_type& c) : data_queue(c) {}
	template<typename _InputIterator>
	threadsafe_queue(_InputIterator first, _InputIterator last)
	{
		for (auto it = first; it != last; ++it) {
			data_queue.push(*it);
		}
	}

	// 使用初始化列表为参数的构造函数
	threadsafe_queue(std::initializer_list<value_type> list) : threadsafe_queue(list.begin(), list.end()) {}

	// 入队列
	void push(const value_type& value)
	{
		std::lock_guard<std::mutex> lk(mtx);
		data_queue.push(std::move(value));
		data_cond.notify_one();
	}

	// 出队列
	value_type wait_and_pop()
	{
		std::unique_lock<std::mutex> lk(mtx);
		data_cond.wait(lk, [this]{return !this->data_queue.empty();});
		auto value = std::move(data_queue.front());
		data_queue.pop();
		return value;
	}

	bool try_pop(value_type& value)
	{
		std::lock_guard<std::mutex> lk(mtx);
		if (data_queue.empty())
			return false;
		value = std::move(data_queue.front());
		data_queue.pop();
		return true;
	}

	// 返回队列是否为空
	auto empty() const->decltype(data_queue.empty())
	{
		std::lock_guard<std::mutex> lk(mtx);
		return data_queue.empty();
	}

	// 返回队列元素个数
	auto size() const->decltype(data_queue.size())
	{
		std::lock_guard<std::mutex> lk(mtx);
		return data_queue.size();
	}
};

#endif // __THREADSAFE_QUEUE_H__