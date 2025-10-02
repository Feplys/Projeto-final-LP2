#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <chrono>

template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_) return;
        queue_.push(std::move(item));
        cv_.notify_one();
    }

    std::optional<T> pop_timeout(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, timeout, [this]{ return !queue_.empty() || shutdown_; })) {
            if (!queue_.empty()) {
                T item = std::move(queue_.front());
                queue_.pop();
                return item;
            }
        }
        return std::nullopt; 
    }
    
    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cv_.notify_all();
    }
};

#endif 


