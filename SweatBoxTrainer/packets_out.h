#ifndef PACKETS_OUT_H
#define PACKETS_OUT_H

#include "usermanager.h"

void sendPositionUpdates(Aircraft& user);

void sendFlightPlan(Aircraft& user);

void updateMode(Aircraft& user);

void updateSquawk(Aircraft& user);

void sendDisconnect(Aircraft& user);

void sendPingPacket(Aircraft& user);

void sendUserMessage(Aircraft& user, int frequency, std::string to, std::string message);

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
_PRIVATE_MESSAGE = 12;

#endif
