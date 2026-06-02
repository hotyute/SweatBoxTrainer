#ifndef GEOUTILS_H 
#define GEOUTILS_H

#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "SweatBoxTrainer/point2d.h"

bool inCircle(Point2& p1, Point2& p2, Point2& c, double radius_meters);

bool inCircle2(const Point2& p1, Point2& p2, Point2& c, double r);

bool pointCircle(double px, double py, double cx, double cy, double r);

bool linePoint(double x1, double y1, double x2, double y2, double px, double py);

bool taxiIntersect(Point2& p, Point2& p2);

#endif
