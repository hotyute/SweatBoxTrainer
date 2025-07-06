#pragma once

#include "constants.h"
#include <chrono>

class EventAction {
private:
	bool running;
	long long ticks;
	std::chrono::steady_clock::time_point lastEvent;
public:
	bool paused = false;

	EventAction() {
		running = true;
		lastEvent = std::chrono::steady_clock::now();
		ticks = 0L;
	}
	void setRunning(bool value) {
		running = value;
	}
	bool getRunning() {
		return running;
	}
	void setLastEvent(std::chrono::steady_clock::time_point value) {
		lastEvent = value;
	}
	std::chrono::steady_clock::time_point getLastEvent() {
		return lastEvent;
	}
	void setTicks(long long value) {
		ticks = value;
	}
	long long getTicks() {
		return ticks;
	}

};

class Event {
public:
	EventAction eAction;
	void* object = nullptr;
	virtual void execute() = 0;
	void toggle_pause() { this->eAction.paused = !this->eAction.paused; }
	virtual void stop() = 0;
};

class EventManager {
private:
	std::vector<Event*> events;
	int eventCount = 0;

/// <summary>
/// We create all event spots Immediately so we can be more "multi-thread compliant
/// </summary>
/// <param name="e"></param>

public:
	EventManager() : events(MAX_EVENTS, nullptr) { }
	void addEvent(Event* e) {
		auto it = std::find(events.begin(), events.end(), nullptr);
		if (it != events.end())
			*it = e;
	}
	bool removeEvent(Event* e) {
		return events.erase(std::find(events.begin(), events.end(), e)) != events.end();
	}
	void update();

};

extern EventManager* event_manager1;