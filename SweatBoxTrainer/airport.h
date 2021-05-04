#ifndef AIRPORT_H
#define AIRPORT_H

#include <iostream>
#include <vector>

#include "point2d.h"

class Airport 
{
	double elevation;
	std::vector<Point2*> runways;
};

#endif