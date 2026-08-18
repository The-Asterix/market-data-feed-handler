#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

// A generic queue that's safe to push/pop from multiple threads at once.
// Producer calls push(), consumer calls wait_and_pop() -- which blocks
// (sleeps, using zero CPU) until something is available.
template<typename T>
class ThreadSafeQueue {
public:
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(item));
        }
        // Wake up one thread that's sleeping in wait_and_pop(), if any.
        cv_.notify_one();
    }

    // Blocks until an item is available, then removes and returns it.
    // Returns false if shutdown() was called and the queue is empty
    // (this is how we cleanly stop the consumer thread).
    bool wait_and_pop(T& out) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Sleep until EITHER the queue has something, OR we're shutting down.
        // The lambda is checked every time we wake up -- this guards against
        // "spurious wakeups", a rare OS quirk where wait() can return even
        // with nothing to do; the lambda re-checks the real condition.
        cv_.wait(lock, [this] { return !queue_.empty() || shutting_down_; });

        if (queue_.empty() && shutting_down_) {
            return false; // nothing left, and we're stopping -- tell caller to exit
        }

        out = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // Called by the producer when there's no more data coming.
    // Wakes up the consumer even if the queue is empty, so it can exit
    // instead of waiting forever.
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutting_down_ = true;
        }
        cv_.notify_one();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutting_down_ = false;
};
