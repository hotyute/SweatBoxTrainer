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

				/*if (aircraft.getIdentity()->callsign == "DAL220") 
				{
					Point2 p = Point2(-080.30143039, 25.80070055);
					Point2 p2 = Point2(-080.26953539, 25.80201755);
					double h = degrees(GetHeading(aircraft.getLatitude(), p2.y_, aircraft.getLongitude(), p2.x_));
					av.asgd_heading = hdg(h - GetCTE2(p, p2, aircraft.getLatitude(), aircraft.getLongitude(), aircraft.getSpeed()));
					std::cout << av.asgd_heading << std::endl;
				}*/

				aircraft.updateSpeed();
				aircraft.updateHeading();
				aircraft.updateMovement();

				aircraft.SetTrackData();
				aircraft.getNextPoint();

			}
		}
	}
}
