#include "events.h"

#include "packets_out.h"
#include "usermanager.h"

PositionUpdates::PositionUpdates(void* obj)
{
	this->object = obj;
}

//event handler for position updates
void PositionUpdates::execute() {
	Aircraft* aircraft = (Aircraft*)object;
	if (PositionUpdates::eAction.getTicks() == 0
		|| aircraft->getUpdateTime() != PositionUpdates::eAction.getTicks()) {
		PositionUpdates::eAction.setTicks(aircraft->getUpdateTime());
	}
	sendPositionUpdates(*aircraft);
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