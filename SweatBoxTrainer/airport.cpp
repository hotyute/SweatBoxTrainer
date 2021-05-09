#include "airport.h"

#include "tools.h"

std::unordered_map<std::string, Airport*> airports;

Point2* TaxiPath::getNextPoint(Point2* last)
{
	int p_index = -1;

	double last_dist = -1;
	Point2* p2add = nullptr;
	for (size_t i = 0; i < points.size(); ++i)
	{
		Point2* p = points[i];
		if (p)
		{
			double cur_dist = dist(last->y_, last->x_, p->y_, p->x_);
			if (last_dist == -1 || cur_dist < last_dist) 
			{
				last_dist = cur_dist;
				p2add = p;
				p_index = i;
			}
		}
	}
	return p2add;
}

Point2* TaxiPath::getClosestPoint(double latitude, double longitude)
{
	return new Point2();
}
