#pragma once

#include "../tools/timed_task.h"

// This task is responsible for periodically sending a ping packet to all connected clients.
class PingTask : public TimedTask {
public:
    // The constructor sets up the task to run on a 10-second interval.
    explicit PingTask(ThreadPool& pool);

protected:
    // This is the core logic that will be executed by the thread pool.
    void execute() override;
};