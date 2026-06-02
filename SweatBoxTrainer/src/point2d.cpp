#include "SweatBoxTrainer/point2d.h"

double* Point2::as_array()
{
	Point2::p[0] = x_;
	Point2::p[1] = y_;
	Point2::p[2] = 0;
	return Point2::p;
}