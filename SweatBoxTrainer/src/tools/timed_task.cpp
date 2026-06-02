#include "SweatBoxTrainer/tools/timed_task.h"
#include <algorithm> // For std::remove

// Static member initialization
std::vector<TimedTask*> TimedTask::s_allTasks;
std::mutex TimedTask::s_tasksMutex; 
std::mutex TimedTask::s_cvMutex;
std::thread TimedTask::s_schedulerThread;
std::atomic<bool> TimedTask::s_schedulerRunning(false);
std::condition_variable TimedTask::s_schedulerCv;


TimedTask::TimedTask(ThreadPool& threadPool, std::chrono::milliseconds interval, bool compensate)
    : m_threadPool(threadPool), m_interval(interval),
    m_compensateComputeTime(compensate), m_isRunning(false),
    m_lastExecutionTime(std::chrono::steady_clock::now()) // Initialize last execution time
{
}

TimedTask::~TimedTask() {
    stop(); // Ensure task is removed from global list if active
}

void TimedTask::start() {
    if (m_isRunning.exchange(true)) return; // Atomically set to true, if already true, return
    m_lastExecutionTime = std::chrono::steady_clock::now();
    {
        std::unique_lock<std::mutex> lock(s_tasksMutex);
        auto it = std::find(s_allTasks.begin(), s_allTasks.end(), this);
        if (it == s_allTasks.end()) {
            s_allTasks.push_back(this);
        }
    } // s_tasksMutex is released
    startSchedulerThread(); // Ensure scheduler is running
    s_schedulerCv.notify_one(); // Notify scheduler to re-evaluate wait times, potentially for the new task
}

void TimedTask::stop() {
    if (!m_isRunning) return;
    m_isRunning = false;
    {
        std::unique_lock<std::mutex> lock(s_tasksMutex);
        s_allTasks.erase(std::remove(s_allTasks.begin(), s_allTasks.end(), this), s_allTasks.end());
    }
    // If s_allTasks is empty, the scheduler thread might be stopped.
    // Consider logic to stop scheduler if no tasks are running.
}

bool TimedTask::isDue(const std::chrono::steady_clock::time_point& currentTime) const {
    if (!m_isRunning) return false;
    return currentTime >= (m_lastExecutionTime + m_interval);
}

void TimedTask::poll_socket() {
    if (!m_isRunning) return;

    auto startTime = std::chrono::steady_clock::now();
    m_threadPool.enqueue([this, startTime]() { // Capture startTime for compensation
        this->execute(); // Call the derived class's execute method

        // Update last execution time
        if (this->m_compensateComputeTime) {
            // This simple compensation might lead to drift if tasks overrun significantly.
            // A more robust system might use a fixed schedule relative to the start time.
            this->m_lastExecutionTime = startTime;
        }
        else {
            this->m_lastExecutionTime = std::chrono::steady_clock::now();
        }
        });
    // If not compensating, m_lastExecutionTime should be set here,
    // or after enqueue if execute() is very short.
    // For now, let the thread update it to reflect when it *actually* finished or started.
    if (!m_compensateComputeTime) {
        m_lastExecutionTime = startTime; // Mark as poll_socket, next interval from this point
    }
}


// --- Static Scheduler Methods ---
void TimedTask::startSchedulerThread() {
    if (!s_schedulerRunning.exchange(true)) { // Atomically set to true if it was false
        s_schedulerThread = std::thread(TimedTask::schedulerLoop);
    }
}

void TimedTask::stopSchedulerThread() {
    if (s_schedulerRunning.exchange(false, std::memory_order_release)) { // Use release
        s_schedulerCv.notify_one(); // Wakes up scheduler to check the new s_schedulerRunning state
        if (s_schedulerThread.joinable()) {
            s_schedulerThread.join();
        }
    }
}

void TimedTask::schedulerLoop() {
    while (s_schedulerRunning.load(std::memory_order_acquire)) { // Use acquire for visibility
        std::chrono::milliseconds shortestWaitTime = std::chrono::milliseconds::max();
        bool anyTasksActive = false; // Renamed for clarity

        // --- Section 1: Access s_allTasks (needs s_tasksMutex) ---
        { // Scope for s_tasksMutex
            std::unique_lock<std::mutex> tasks_lock(s_tasksMutex);
            auto now = std::chrono::steady_clock::now();

            for (TimedTask* task : s_allTasks) {
                if (task && task->m_isRunning) {
                    anyTasksActive = true;
                    if (task->isDue(now)) {
                        task->poll_socket();
                    }
                    // Calculate time until next execution for this task
                    auto nextExecTime = task->m_lastExecutionTime + task->m_interval;
                    if (nextExecTime > now) {
                        auto waitTime = std::chrono::duration_cast<std::chrono::milliseconds>(nextExecTime - now);
                        shortestWaitTime = std::min(shortestWaitTime, waitTime);
                    }
                    else {
                        // Task is overdue or just ran, its effective wait time for next check is its interval,
                        // or 0 if we want to re-evaluate immediately. Let's keep it its interval
                        // or a very small value if we want to immediately re-check.
                        shortestWaitTime = std::min(shortestWaitTime, std::max(std::chrono::milliseconds(0), task->m_interval));
                    }
                }
            }
        } // s_tasksMutex (tasks_lock) is released here

        // --- Section 2: Conditional Variable Wait (uses s_cvMutex) ---
        if (!anyTasksActive && s_allTasks.empty() && s_schedulerRunning.load(std::memory_order_acquire)) {
            // No active tasks and list is empty, scheduler can pause for a default duration or wait for notification.
            std::unique_lock<std::mutex> cv_wait_lock(s_cvMutex);
            s_schedulerCv.wait_for(cv_wait_lock, std::chrono::milliseconds(100),
                [] { return !s_schedulerRunning.load(std::memory_order_acquire); });
            continue; // Re-evaluate loop
        }

        if (shortestWaitTime == std::chrono::milliseconds::max() || shortestWaitTime.count() <= 0) {
            // If all tasks just ran or are overdue, or no tasks, wait a small default time.
            shortestWaitTime = std::chrono::milliseconds(10);
        }
        shortestWaitTime = std::max(shortestWaitTime, std::chrono::milliseconds(1)); // Ensure wait time is positive

        std::unique_lock<std::mutex> cv_wait_lock(s_cvMutex);
        s_schedulerCv.wait_for(cv_wait_lock, shortestWaitTime,
            [] { return !s_schedulerRunning.load(std::memory_order_acquire); });
    }
}