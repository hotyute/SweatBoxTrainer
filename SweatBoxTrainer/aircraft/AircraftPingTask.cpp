// --- START OF FILE AircraftPingTask.cpp ---

#include "AircraftPingTask.h"
#include "aircraft.h"
#include "../packets_out.h"

// Let's define the ping interval as a constant.
constexpr long PING_INTERVAL_MS = 10000;

AircraftPingTask::AircraftPingTask(ThreadPool& pool, Aircraft& aircraft)
    : TimedTask(pool, std::chrono::milliseconds(PING_INTERVAL_MS), false),
    m_aircraft(aircraft)
{
}

void AircraftPingTask::execute()
{
    // The logic is now very simple: just ping our one aircraft if it's connected.
    if (m_aircraft.connected) {
        sendPingPacket(m_aircraft);
    }
}