#include "airport.h"

#include "tools.h"

std::unordered_map<std::string, Airport*> airports;

Point2* TaxiPath::getNextPoint(Point2* p)
{
	double last_dist = -1;
	Point2* p2add = nullptr;
	for (size_t i = 0; i < points.size(); ++i)
	{
		Point2* p2 = points[i];
		if (p)
		{
			if (*p != *p2)//compares x and y
			{
				double cur_dist = dist(p->y_, p->x_, p2->y_, p2->x_);
				if (last_dist == -1 || cur_dist < last_dist)
				{
					last_dist = cur_dist;
					p2add = p2;
				}
			}
		}
	}
	return p2add;
}

Point2* TaxiPath::getClosestPoint(double latitude, double longitude)
{
	return getNextPoint(&Point2(longitude, latitude));
}
