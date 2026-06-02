#include "SweatBoxTrainer/tools/thread_pool.h"

#include <iostream>

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    if (numThreads == 0) numThreads = 1; // Ensure at least one worker
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ThreadPool::workerLoop, this); // FIX: Renamed worker to workerLoop
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(tasksMutex);
        stop = true;
    }
    condition.notify_all();

    for (std::thread& worker_thread : workers) { // FIX: Variable name more descriptive
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
}

void ThreadPool::enqueue(std::function<void()> task) { // FIX: pass by value or const ref + move
    if (stop) {
        // Optionally throw an exception or log if trying to enqueue to a stopped pool
        // throw std::runtime_error("enqueue on stopped ThreadPool");
        return;
    }
    {
        std::unique_lock<std::mutex> lock(tasksMutex);
        tasks.push(std::move(task)); // Use std::move for efficiency
    }
    condition.notify_one();
}

void ThreadPool::workerLoop() { // FIX: Renamed from worker
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(tasksMutex);
            condition.wait(lock, [this]() { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) {
                return; // Exit loop
            }
            if (tasks.empty()) { // Should not happen if stop is false due to predicate
                continue;
            }

            task = std::move(tasks.front()); // Use std::move
            tasks.pop();
        }
        try {
            task(); // Execute the task
        }
        catch (const std::exception& e) {
            // Log exception or handle it appropriately
            std::cerr << "Exception in thread pool worker: " << e.what() << std::endl;
        }
        catch (...) {
            // Handle other types of exceptions
            std::cerr << "Unknown exception in thread pool worker." << std::endl;
        }
    }
}