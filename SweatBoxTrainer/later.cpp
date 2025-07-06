#include "later.h"


EventManager* event_manager1 = new EventManager();

void EventManager::update() {
	for (auto it = EventManager::events.begin(); it != EventManager::events.end();) 
	{
		Event* _event = *it;
		if (_event != nullptr)
		{
			if (!_event->eAction.getRunning())
			{
				delete _event;
				it = EventManager::events.erase(it);
				eventCount--;
				continue;
			}

			if (!_event->eAction.paused)
			{
				auto now = std::chrono::steady_clock::now();
				auto duration_since_last = now - _event->eAction.getLastEvent();
				long long mills = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_last).count();
				if (mills >= _event->eAction.getTicks()) {
					_event->execute();
					_event->eAction.setLastEvent(std::chrono::steady_clock::now());
				}
			}
		}
		it++;
	}
}