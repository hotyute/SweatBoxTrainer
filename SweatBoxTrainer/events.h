#pragma once

#include "later.h"

class PositionUpdates : public Event {
public:
	PositionUpdates(void* obj);
	void execute();
	void stop();
};

class GraphicsUIUpdates : public Event {
public:
	void execute();
	void stop();
};