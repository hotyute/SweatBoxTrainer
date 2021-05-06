#ifndef USERMANAGER_H
#define USERMANAGER_H

const int PILOT_CLIENT_UPDATE_PACKET = 1,
CLIENT_TRANSPONDER_PACKET = 2,
CONTROLLER_CLIENT_UPDATE_PACKET = 3,
CLIENT_PONG_PACKET = 4,
CLIENT_CHANGE_REQ_CYCLE = 5,
MESSAGE_RECEIVED = 6,
PILOT_CLIENT_TITLE = 7,
CLIENT_MODE_PACKET = 8,
CLIENT_DISCONNECT_PACKET = 9,
FLIGHT_PLAN_UPDATE_PACKET = 10,
REQUEST_FLIGHT_PLAN_PACKET = 11,
PRIVATE_MESSAGE_PACKET = 12;

#include <vector>
#include <unordered_map>

#include "aircraft.h"
#include "Stream.h"

extern std::vector<Aircraft*> userStorage1;

void processIncomingPackets(Aircraft* aircraft, int opCode, Stream &stream);

Aircraft* createAircraft(std::string callsign, double latitude, double longitude, double heading, double speed, int altitude,
	int verticalSpeed, int mode, std::string squawkCode);

#endif
