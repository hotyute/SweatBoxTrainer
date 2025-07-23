// --- START OF FILE AircraftPingTask.h ---

#pragma once

#include "../tools/timed_task.h"

// Forward declaration
class Aircraft;

// This task is created for a SINGLE aircraft and sends a ping on a fixed interval.
class AircraftPingTask : public TimedTask {
public:
    explicit AircraftPingTask(ThreadPool& pool, Aircraft& aircraft);

protected:
    void execute() override;

private:
    Aircraft& m_aircraft; // A reference to the specific aircraft this task serves
};

// --- END OF FILE AircraftPingTask.h ---