#include "usermanager.h"

#include "SweatBoxTrainer.h"
#include "tools.h"
#include "packets_out.h"
#include "globals.h"

std::vector<Aircraft*> userStorage1;

Aircraft* createAircraft(std::string callsign, double latitude, double longitude, double heading, double speed, double altitude,
	double verticalSpeed, int mode, std::string squawkCode)
{
	// 1. Create the new aircraft object. It contains all its own components.
	auto cur = std::make_unique<Aircraft>();

	// 2. Set high-level properties and simple data members
	cur->setType(AV_CLIENT::PILOT);
	cur->setHeavy(false);
	cur->setMode(mode);
	cur->setSquawkCode(squawkCode);

	// 3. Get references to the internal components for initialization
	Identity& id = *cur->getIdentity();
	AircraftState& state = cur->getState();
	AssignedValues& av = cur->getAssignedValues();

	// 4. Initialize the Identity component
	id.callsign = callsign;
	id.username = "971202";
	id.login_name = "Samuel Mason";
	id.password = "password";
	id.pilot_rating = 1;

	// 5. Initialize the AircraftState component with the starting physical values
	state.latitude = latitude;
	state.longitude = longitude;
	state.altitude = altitude;
	state.speed = speed;
	state.heading = heading;
	state.verticalSpeed = verticalSpeed;
	// The state's dirty flags are already set to ALL by default, so the GUI will update.

	// 6. Initialize the AssignedValues component.
	// This is crucial so the aircraft doesn't immediately try to change its state.
	av.asdg_altitude = altitude;
	av.asdg_speed = speed;
	av.asgd_heading = heading;
	// Other assigned values (roll, pitch, etc.) will use their defaults.

	// 7. Initialize the network connection
	cur->getConnection().init_set();

	// 8. Add the new aircraft to the global collections
	Aircraft* raw_ptr = cur.get();
	{
		// The lock correctly protects the shared AcfMap collection
		std::lock_guard<std::mutex> lock(g_acfMapMutex);
		AcfMap[callsign] = std::move(cur); // Move ownership into the map
	}

	addUserToLB(raw_ptr);
	return raw_ptr;
}
