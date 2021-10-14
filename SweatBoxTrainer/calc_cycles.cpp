#include "calc_cycles.h"

#include <stdio.h>
#include <math.h>

#include "usermanager.h"
#include "tools.h"

void update()
{
}

void CalculateMovements()
{
	if (AcfMap.size() > 0) {
		for (auto iter = AcfMap.begin(); iter != AcfMap.end(); ++iter)
		{
			Aircraft* acf1 = iter->second;
			if (acf1) {
				Aircraft& aircraft = *acf1;



				AssignedValues& av = aircraft.getAssignedValues();

				aircraft.CollisionDetection();
				aircraft.CheckFrameFlags();

				aircraft.updateSpeed();
				aircraft.updateHeading();
				aircraft.updateMovement();
				aircraft.updateAltitude();

				if (aircraft.getSpeed() > 0 && (!aircraft.holding() && !aircraft.idle()))
					aircraft.getNextPoint();

			}
		}
	}
}
