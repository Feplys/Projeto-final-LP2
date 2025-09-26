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
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;
    
    void push(T item);
    
    bool pop(T& item);
    
    std::optional<T> pop_timeout(std::chrono::milliseconds timeout);
    
    bool empty() const;
    
    size_t size() const;
    
    void shutdown();
    
    // Não copiável
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
};

#include "thread_safe_queue.tpp" // Implementação do template

#endif // THREAD_SAFE_QUEUE_H
