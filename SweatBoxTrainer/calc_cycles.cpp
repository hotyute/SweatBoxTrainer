#include "calc_cycles.h"

#include <stdio.h>
#include <math.h>

#include "usermanager.h"
#include "packets_out.h"
#include "tools.h"
#include "globals.h"
#include "aircraft/Aircraft.h"

void update()
{
}

void SimulationTask::execute() {
	// The core simulation logic goes here.
	CalculateMovements();

	// The ping logic needs to be adapted. It shouldn't be tied to a wall-clock check inside the loop.
	// A separate TimedTask is a much better design for this.
}

void CalculateMovements()
{
	std::lock_guard<std::mutex> lock(g_acfMapMutex); // Lock the mutex
	if (AcfMap.size() > 0) {
		for (auto const& [key, acf_ptr] : AcfMap)
		{
			if (acf_ptr) {
				Aircraft& aircraft = *acf_ptr;

				aircraft.update();

				// --- MERGED LOGIC FROM PositionUpdates ---
				// After all calculations are done for this aircraft, send its update.
				//if (aircraft.connected) {
				//	sendPositionUpdates(aircraft);
				//}

			}
		}
	}
}
