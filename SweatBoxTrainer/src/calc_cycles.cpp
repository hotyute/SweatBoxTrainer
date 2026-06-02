#include "SweatBoxTrainer/calc_cycles.h"

#include <stdio.h>
#include <math.h>

#include "SweatBoxTrainer/usermanager.h"
#include "SweatBoxTrainer/packets_out.h"
#include "SweatBoxTrainer/tools.h"
#include "SweatBoxTrainer/globals.h"
#include "SweatBoxTrainer/aircraft/aircraft.h"
#include "SweatBoxTrainer/sim/simulation_context.h"

void update()
{
}

void SimulationTask::execute() {
	// The core simulation logic goes here.
	CalculateMovements();

	// The ping logic has been moved to its own dedicated PingTask.
	// No more clock-checking here!
}

void CalculateMovements()
{
	auto& ctx = SimulationContext::instance();
	std::lock_guard<std::mutex> lock(ctx.aircraftMutex());
	for (auto& [callsign, acPtr] : ctx.aircraft())
	{
		Aircraft& aircraft = *acPtr;

		aircraft.update();

		// --- MERGED LOGIC FROM PositionUpdates ---
		// After all calculations are done for this aircraft, send its update.
		//if (aircraft.connected) {
		//	sendPositionUpdates(aircraft);
		//}
	}
}
