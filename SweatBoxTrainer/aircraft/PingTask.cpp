#include "PingTask.h"

#include "../sim/simulation_context.h"
#include "../packets_out.h"
#include "../aircraft/Aircraft.h"
#include "../guidialogue.h" // For logging (optional)

// The constructor calls the base TimedTask constructor, setting the interval to 10000ms.
// We set 'compensate' to false because we want the interval to be a fixed 10 seconds,
// regardless of how long the ping process takes.
PingTask::PingTask(ThreadPool& pool)
    : TimedTask(pool, std::chrono::milliseconds(10000), false)
{
}

void PingTask::execute() {
    auto& ctx = SimulationContext::instance();

    // Acquire a lock on the aircraftMutex. This is crucial to prevent race conditions
    // with other threads that might be adding, removing, or modifying aircraft.
    // The lock is automatically released when 'lock' goes out of scope.
    std::lock_guard<std::mutex> lock(ctx.aircraftMutex());

    if (ctx.aircraft().empty()) {
        return; // No aircraft to ping, just return.
    }

    // Optional: Log that the task is running.
    // AppendTextToConsole(L"Sending ping to all connected clients...");

    // Use a modern range-based for loop to iterate over every aircraft in the map.
    for (const auto& [callsign, acf_ptr] : ctx.aircraft())
    {
        // The unique_ptr (acf_ptr) should always be valid, but this is a good safety check.
        if (acf_ptr) {
            Aircraft& aircraft = *acf_ptr;

            // Only send pings to clients that are actually connected.
            if (aircraft.connected) {
                sendPingPacket(aircraft);
            }
        }
    }
}