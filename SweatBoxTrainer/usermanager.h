#pragma once

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
#include <string>
#include <memory> // For std::unique_ptr

#include "aircraft.h"
#include "basic_stream.h"

void disconnect(Aircraft* aircraft, bool queue);

// This vector stores non-owning pointers. Be careful with lifetime.
// It seems to be a cache for index-based lookup.
extern std::vector<Aircraft*> userStorage1;

void processIncomingPackets(Aircraft* aircraft, int opCode, BasicStream& stream);

// The function now returns a unique_ptr, transferring ownership.
std::unique_ptr<Aircraft> createAircraft(std::string callsign, double latitude, double longitude, double heading, double speed, double altitude,
	double verticalSpeed, int mode, std::string squawkCode);

const int command_freq = 18300, msg_freq = 99998;

int processCommands(Aircraft& aircraft, std::string text);
