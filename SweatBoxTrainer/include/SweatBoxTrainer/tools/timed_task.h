#pragma once

#include "SweatBoxTrainer/tools/thread_pool.h"
#include <chrono>
#include <atomic>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable> // For scheduler CV

class TimedTask {
public:
    // interval: how often the task should poll_socket
    // compensateComputeTime: if true, tries to maintain interval regardless of task duration (can lead to bursts)
    TimedTask(ThreadPool& threadPool, std::chrono::milliseconds interval, bool compensateComputeTime = false);
    virtual ~TimedTask();

    TimedTask(const TimedTask&) = delete;
    TimedTask& operator=(const TimedTask&) = delete;

    void start(); // Adds task to scheduler and marks as running
    void stop();  // Removes task from scheduler and marks as not running

    bool isRunning() const { return m_isRunning; }
    std::chrono::milliseconds getInterval() const { return m_interval; }

    // Static methods to control the global scheduler thread
    static void startSchedulerThread();
    static void stopSchedulerThread(); // Call this at application shutdown

protected:
    virtual void execute() = 0; // The actual work to be done by the task

private:
    friend class Scheduler; // Allow a dedicated scheduler class to access private members if you create one

    ThreadPool& m_threadPool;
    std::chrono::milliseconds m_interval;
    bool m_compensateComputeTime;
    std::atomic<bool> m_isRunning;
    std::chrono::steady_clock::time_point m_lastExecutionTime;

    // Called by the scheduler when the task is due
    void poll_socket();
    bool isDue(const std::chrono::steady_clock::time_point& currentTime) const;

    // Static members for the global scheduler
    static std::vector<TimedTask*> s_allTasks;
    static std::mutex s_tasksMutex; // Protects s_allTasks
    static std::mutex s_cvMutex; // Dedicated mutex for the condition variable
    static std::thread s_schedulerThread;
    static std::atomic<bool> s_schedulerRunning;
    static std::condition_variable s_schedulerCv; // For waking up scheduler

    static void schedulerLoop(); // The function poll_socket by s_schedulerThread
};