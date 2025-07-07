#include "calc_cycles.h"

#include <stdio.h>
#include <math.h>

#include "usermanager.h"
#include "packets_out.h"
#include "tools.h"
#include "globals.h"

void update()
{
}

void SimulationTask::execute() {
	// The core simulation logic goes here.
	// This replaces the old CalcThread1 loop body.
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



				AssignedValues& av = aircraft.getAssignedValues();

				aircraft.CollisionDetection();
				aircraft.CheckFrameFlags();

				aircraft.updateSpeed();
				aircraft.updateRoll();
				aircraft.updatePitch();
				aircraft.updateHeading();
				aircraft.updateMovement();
				aircraft.updateAltitude();
				// V-Speed is not updated in a dedicated method, so we set it here if needed
				// For now, let's assume V-Speed is constant or changed by commands.

				if (aircraft.getSpeed() > 0 && (!aircraft.holding() && !aircraft.idle()))
					aircraft.getNextPoint();

				// --- MERGED LOGIC FROM PositionUpdates ---
				// After all calculations are done for this aircraft, send its update.
				if (aircraft.connected) {
					sendPositionUpdates(aircraft);
				}

			}
		}
	}
}
