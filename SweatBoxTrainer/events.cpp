#include "events.h"

//event handler for position updates
void PositionUpdates::execute() {
	//if (PositionUpdates::eAction.getTicks() == 0
	//	|| USER->getUpdateTime() != PositionUpdates::eAction.getTicks()) {
	//	PositionUpdates::eAction.setTicks(USER->getUpdateTime());
	//}
}

void PositionUpdates::stop() {
}


void ConfigUpdates::execute() {
	//if (ConfigUpdates::eAction.getTicks() == 0
	//	|| USER->getUpdateTime() != ConfigUpdates::eAction.getTicks()) {
	//	ConfigUpdates::eAction.setTicks(1000);
	//}
}

void ConfigUpdates::stop() {
}