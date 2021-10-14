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
				boost::posix_time::ptime mst1 = boost::posix_time::microsec_clock::local_time();
				boost::posix_time::time_duration last_duration = mst1 - _event->eAction.getLastEvent();
				long long mills = last_duration.total_milliseconds();
				if (mills >= _event->eAction.getTicks()) {
					_event->execute();
					_event->eAction.setLastEvent(boost::posix_time::microsec_clock::local_time());
				}
			}
		}
		it++;
	}
}