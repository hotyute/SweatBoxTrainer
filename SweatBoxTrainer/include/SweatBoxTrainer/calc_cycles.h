#pragma once

#include "SweatBoxTrainer/tools/timed_task.h"

void update();

void CalculateMovements();

class SimulationTask : public TimedTask {
public:
    // Run at ~33Hz (30ms interval)
    SimulationTask(ThreadPool& pool)
        : TimedTask(pool, std::chrono::milliseconds(30), true) {} // Compensate for compute time

protected:
    void execute() override;
};

