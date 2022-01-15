#include "airport.h"

#include "tools.h"
#include "geoutils.h"

std::unordered_map<std::string, Airport*> airports;

Point2* TaxiPath::getPrevPoint(Point2* to, Point2* next)
{
	if (to->index < next->index)
	{
		if ((to->index - 1) >= 0)
		{
			return points[to->index - 1];
		}
	}
	else if (to->index > next->index)
	{
		if ((to->index + 1) < points.size())
		{
			return points[to->index + 1];
		}
	}
	return nullptr;
}

Point2* TaxiPath::getNextPoint(Point2* p, Point2* p2)
{
	//std::cout << p->index << ", " << p2->index << std::endl;
	if (p && p2 && p->index != -1 && p2->index != -1)
	{
		if (p->index < p2->index)
		{
			for (int i = p->index; i < p2->index; ++i)
			{
				Point2* p3 = points[i];
				if (p3 && p3 != p)
				{
					return p3;
				}
			}
		}
		else if (p->index > p2->index)
		{
			//std::cout << name << std::endl;
			for (int i = p->index; i > p2->index; --i)
			{
				Point2* p3 = points[i];
				if (p3 && p3 != p)
				{
					return p3;
				}
			}
		}
		else
		{
			return p2;
		}
	}
	return p2;
}

void TaxiPath::getPoints(Point2* p, Point2* p2, std::vector<Point2*>& point_store)
{
	//std::cout << p->index << ", " << p2->index << std::endl;
	if (p && p2 && p->index != -1 && p2->index != -1)
	{
		if (p->index < p2->index)
		{
			for (int i = p->index; i < p2->index; ++i)
			{
				Point2* p3 = points[i];
				if (p3 && p3 != p)
				{
					point_store.push_back(p3);
				}
			}
		}
		else if (p->index > p2->index)
		{
			//std::cout << name << std::endl;
			for (int i = p->index; i > p2->index; --i)
			{
				Point2* p3 = points[i];
				if (p3 && p3 != p)
				{
					point_store.push_back(p3);
				}
			}
		}
	}
}

Point2* TaxiPath::angleTest(Point2* orig, Point2* p, Point2* p2)
{
	Point2* f = p;
	while (isHeavyAngle(orig, f, p2))
	{
		if (f == p2)
			break;
		f = getNextPoint(f, p2);
	}
	return f;
}

Point2* TaxiPath::getClosestPoint(double latitude, double longitude)
{
	Point2 p = Point2(longitude, latitude);
	double last_dist = -1;
	Point2* p2add = nullptr;
	for (size_t i = 0; i < points.size(); ++i)
	{
		Point2* p2 = points[i];
		if (p != *p2)//compares x and y
		{
			double cur_dist = dist(p.y_, p.x_, p2->y_, p2->x_);
			if (last_dist == -1 || cur_dist < last_dist)
			{
				last_dist = cur_dist;
				p2add = p2;
			}
		}
	}
	return p2add;
}

Point2* TaxiPath::getClosest(TaxiPath* next)
{
	double last_dist = -1;
	Point2* p2add = nullptr;
	for (size_t i = 0; i < next->points.size(); ++i)
	{
		Point2* p = next->points[i];
		if (p)
		{
			for (size_t i2 = 0; i2 < points.size(); ++i2)
			{
				Point2* p2 = points[i2];
				if (p2)
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
		}
	}
	return p2add;
}

Point2* TaxiPath::intersect(Point2& p)
{
	double last_dist = -1;
	Point2* p2add = nullptr;
	for (size_t i2 = 0; i2 < points.size(); ++i2)
	{
		Point2* p2 = points[i2];
		if (p2)
		{
			if (p != *p2)//compares x and y
			{
				if (taxiIntersect(p, *p2))
				{
					p2add = p2;
					break;
				}
			}
		}
	}
	return p2add;
}

Point2* TaxiPath::getStart()
{
	return points.front();
}

Point2* TaxiPath::getEnd()
{
	return points.back();
}
