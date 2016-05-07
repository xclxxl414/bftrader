#ifndef SAFE_QUEUE
#define SAFE_QUEUE

#include <condition_variable>
#include <mutex>
#include <queue>

template <class T>
class SafeQueue {
public:
    SafeQueue(void)
    {
    }
    ~SafeQueue(void)
    {
    }
    void enqueue(T* t)
    {
        std::lock_guard<std::mutex> lock(mu_);
        queue_.push(t);
        cv_.notify_one();
    }

    T* dequeue(void)
    {
        std::unique_lock<std::mutex> lock(mu_);
        while (!shutdown_ && queue_.empty()) {
            cv_.wait(lock);
        }
        if (shutdown_) {
            while (!queue_.empty()) {
                T* val = queue_.front();
                queue_.pop();
                delete val;
            }
            return nullptr;
        } else {
            T* val = queue_.front();
            queue_.pop();
            return val;
        }
    }

    void shutdown(void)
    {
        std::lock_guard<std::mutex> lock(mu_);
        shutdown_ = true;
        cv_.notify_one();
    }

private:
    std::queue<T*> queue_;
    mutable std::mutex mu_;
    std::condition_variable cv_;
    bool shutdown_ = false;
};
#endif
