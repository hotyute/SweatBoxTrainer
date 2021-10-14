#include "events.h"

#include "packets_out.h"
#include "usermanager.h"

/// <summary>
/// Create the PositionUpdate Event
/// </summary>
/// <param name="obj"></param>
PositionUpdates::PositionUpdates(void* obj)
{
	this->object = obj;
}

/// <summary>
/// Where the code gets executed
/// </summary>
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