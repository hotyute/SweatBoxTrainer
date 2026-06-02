#pragma once

#include "SweatBoxTrainer/aircraft/aircraft.h"
#include "SweatBoxTrainer/usermanager.h"

void processIncomingPackets(Aircraft* aircraft, int opCode, BasicStream& stream);

// The new initializer function we will call once at startup.
void initializePacketHandlers();