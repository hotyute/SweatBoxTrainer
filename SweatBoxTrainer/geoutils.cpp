#include "geoutils.h"

#include "tools.h"
#include "constants.h"

#include <windows.h>
#include <algorithm> 
#include <bitset>
#include <functional>
#include <cctype>
#include <locale>
#include <iostream>
#include <sstream>

bool inCircle(Point2& p1, Point2& p2, Point2& c, double r) {
	double dx = p2.x_ = p2.x_;
	double dy = p2.y_ - p1.y_;
	double a = dx * dx + dy * dy;
	double b = 2 * (dx * (p1.x_ - c.x_) + dy * (p1.y_ - c.y_));
	double c1 = c.x_ * c.x_ + c.y_ * c.y_;
	c1 += p1.x_ * p1.x_ + p1.y_ * p1.y_;
	c1 -= 2 * (c.x_ * p1.x_ + c.y_ * p1.y_);
	c1 -= r * r;
	double bb4ac = b * b - 4 * a * c1;
	return bb4ac >= 0;
}

bool inCircle2(Point2& p1, Point2& p2, Point2& c, double r) {
	double dx = p2.x_ = p2.x_;
	double dy = p2.y_ - p1.y_;
	double len = sqrt((dx * dx) + (dy * dy));

	double dot = (((c.x_ - p1.x_) * (p2.x_ - p1.x_)) + ((c.y_ - p1.y_) * (p2.y_ - p1.y_))) / pow(len, 2);

	double closestX = p1.x_ + (dot * (p2.x_ - p1.x_));
	double closestY = p1.y_ + (dot * (p2.y_ - p1.y_));

	boolean onSegment = linePoint(p1.x_, p1.y_, p2.x_, p2.y_, closestX, closestY);
	if (!onSegment) return false;

	// get distance to closest point
	dx = closestX - c.x_;
	dy = closestY - c.y_;
	double distance = sqrt((dx * dx) + (dy * dy));

	if (distance <= r) {
		return true;
	}

	return false;
}

// POINT/CIRCLE
bool pointCircle(double px, double py, double cx, double cy, double r) {

	// get distance between the point and circle's center
	// using the Pythagorean Theorem
	double distX = px - cx;
	double distY = py - cy;
	double distance = sqrt((distX * distX) + (distY * distY));

	// if the distance is less than the circle's
	// radius the point is inside!
	if (distance <= r) {
		return true;
	}
	return false;
}

// LINE/POINT
bool linePoint(double x1, double y1, double x2, double y2, double px, double py) {

	// get distance from the point to the two ends of the line
	double d1 = dist2(px, py, x1, y1);
	double d2 = dist2(px, py, x2, y2);

	// get the length of the line
	double lineLen = dist2(x1, y1, x2, y2);

	// since doubles are so minutely accurate, add
	// a little buffer zone that will give collision
	double buffer = 0.1;    // higher # = less accurate

	// if the two distances are equal to the line's
	// length, the point is on the line!
	// note we use the buffer here to give a range,
	// rather than one #
	if (d1 + d2 >= lineLen - buffer && d1 + d2 <= lineLen + buffer) {
		return true;
	}
	return false;
}

bool taxiIntersect(Point2& p, Point2& p2)
{
	return GetDistance(&p, &p2) <= (50 / KNOTS_FT);
}
