#include "later.h"


EventManager *event_manager1 = new EventManager();

void EventManager::update() {
	std::vector<Event*>::iterator it;
	for (it = EventManager::events.begin(); it != EventManager::events.end();) {
		Event *event1 = *it;
		if (event1 != NULL) {
			if (!event1->eAction.getRunning()) {
				delete * it;
				it = EventManager::events.erase(it);
			} else {
				boost::posix_time::ptime mst1 = boost::posix_time::microsec_clock::local_time();
				boost::posix_time::time_duration last_duration = mst1 - event1->eAction.getLastEvent();
				long long mills = last_duration.total_milliseconds();
				if (mills >= event1->eAction.getTicks()) {
					event1->execute();
					event1->eAction.setLastEvent(boost::posix_time::microsec_clock::local_time());
				}
				it++;
			}
		} else {
			delete * it;
			it = EventManager::events.erase(it);
		}
	}
}