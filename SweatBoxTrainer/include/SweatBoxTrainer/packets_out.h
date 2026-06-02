#pragma once

#include "SweatBoxTrainer/usermanager.h"
#include "SweatBoxTrainer/tools/timed_task.h"

void sendPositionUpdates(Aircraft& user);
void sendFlightPlan(Aircraft& user);
void updateMode(Aircraft& user);
void updateSquawk(Aircraft& user);
void sendDisconnect(Aircraft& user);
void sendPingPacket(Aircraft& user);
void sendUserMessage(Aircraft& user, int frequency, std::string to, std::string message);
void sendTempData(Aircraft& user, std::string& assembly, const void* data, ...);


// --- NEW INSTANCED TASK CLASS ---
// This task is created for a SINGLE aircraft and runs on that aircraft's specific interval.
class AircraftPositionUpdateTask : public TimedTask {
public:
    AircraftPositionUpdateTask(ThreadPool& pool, Aircraft& aircraft);

    // Override the stop method to handle self-deletion
    void stop();

protected:
    void execute() override;

private:
    Aircraft& m_aircraft; // A reference to the specific aircraft this task serves
    bool m_shouldDelete = false; // Flag for safe self-destruction
};
// --- END OF NEW TASK CLASS ---

const int _AIRCRAFT_POS_UPDATE = 1,
_UPDATE_TRANSPONDER = 2,
CONTROLLER_POS_UPDATE = 3,
_UPDATE_MODE = 8,
_PING = 4,
_USER_MESSAGE = 6,
_RECV_TIME_CHANGE = 5,
_SEND_TITLE = 7,
_DISCONNECT_PACKET = 9,
_SEND_FLIGHT_PLAN = 10,
_FLIGHT_PLAN_REQ = 11,
_PRIVATE_MESSAGE = 12,
_PRIMARY_FREQ = 13,
_TEMP_DATA = 14;
