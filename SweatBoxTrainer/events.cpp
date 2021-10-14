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
		|| aircraft->getUpdateTime() != PositionUpdates::eAction.getTicks()) 
	{
		PositionUpdates::eAction.setTicks(aircraft->getUpdateTime());
	}
	sendPositionUpdates(*aircraft);
}

void PositionUpdates::stop() {
}


void GraphicsUIUpdates::execute() {
	if (GraphicsUIUpdates::eAction.getTicks() == 0) {
		GraphicsUIUpdates::eAction.setTicks(100);
	}
	DisplayAircraft();
}

void GraphicsUIUpdates::stop() {
}