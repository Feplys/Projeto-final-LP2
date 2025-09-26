#ifndef THREAD_SAFE_QUEUE_TPP
#define THREAD_SAFE_QUEUE_TPP

#include <stdexcept>

template<typename T>
void ThreadSafeQueue<T>::push(T item) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (shutdown_) {
        throw std::runtime_error("Não é possível adicionar item a uma fila em shutdown.");
    }
    queue_.push(std::move(item));
    cv_.notify_one();
}

template<typename T>
bool ThreadSafeQueue<T>::pop(T& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { 
        return !queue_.empty() || shutdown_; 
    });
    
    if (shutdown_ && queue_.empty()) {
        return false;
    }
    
    item = std::move(queue_.front());
    queue_.pop();
    return true;
}

template<typename T>
std::optional<T> ThreadSafeQueue<T>::pop_timeout(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (cv_.wait_for(lock, timeout, [this]() { return !queue_.empty() || shutdown_; })) {
        if (shutdown_ && queue_.empty()) {
            return std::nullopt;
        }
        
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }
    
    return std::nullopt; // Timeout
}

template<typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template<typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

template<typename T>
void ThreadSafeQueue<T>::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    shutdown_ = true;
    cv_.notify_all();
}

#endif // THREAD_SAFE_QUEUE_TPP
