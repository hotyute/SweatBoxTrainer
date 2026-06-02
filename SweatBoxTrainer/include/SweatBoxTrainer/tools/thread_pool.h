#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future> // For std::future if enqueue returns it
#include <utility> // For std::move

class ThreadPool {
public:
    ThreadPool(size_t numThreads = std::thread::hardware_concurrency()); // Default to hardware concurrency
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = default; // Enable move
    ThreadPool& operator=(ThreadPool&&) = default;

    // Enqueue a task that returns void
    void enqueue(std::function<void()> task); // FIX: pass by value or const ref + move

    // Template version to enqueue tasks that return a value and get a std::future
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;


private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks; // FIX: Store packaged_task for future-returning version

    std::mutex tasksMutex;
    std::condition_variable condition;
    std::atomic<bool> stop; // FIX: Use std::atomic for thread-safe stop flag

    void workerLoop(); // FIX: Renamed from worker
};


// Template implementation must be in the header
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    if (stop) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
    }

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(tasksMutex);
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}