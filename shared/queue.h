#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue(size_t maxSize) : maxSize_(maxSize) {}

    void enqueue(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return queue_.size() < maxSize_; });
        queue_.push(item);
        cv_.notify_all();
    }

    T dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        T item = queue_.front();
        queue_.pop();
        cv_.notify_all();
        return item;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    bool isFull() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size() >= maxSize_;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    size_t maxSize_;
};

#endif // THREAD_SAFE_QUEUE_H
