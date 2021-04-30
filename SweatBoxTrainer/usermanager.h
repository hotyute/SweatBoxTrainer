#ifndef USERMANAGER_H
#define USERMANAGER_H

#define MAX_AIRCRAFT_SIZE 1024

#include <vector>
#include <unordered_map>

#include "aircraft.h"
#include "Stream.h"

extern std::vector<Aircraft*> userStorage1;
extern std::unordered_map<std::string, Aircraft*> users_map;

void decodePackets(Aircraft* aircraft, int opCode, Stream &stream);

#endif
