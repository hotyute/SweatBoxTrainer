#ifndef __TOOLS_H
#define __TOOLS_H

#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "point2d.h"

std::vector<std::string> split(const std::string&, const std::string&, int times);

std::vector<std::string> split(const std::string& s, const std::string& delim);

char* s2ca1(const std::string& s);

int random(int start, int end);

long long doubleToRawBits(double x);

Point2 getLocFromBearing(double latitude, double longitude, double distance, double bearing);

double get_distance(double speed, double interval_ms);

double radians(double degrees);

double degrees(double radians);

double dist(double lat1, double lon1, double lat2, double lon2);

double round_up(double value, int decimal_places);

std::wstring s2ws(const std::string& s);

double atodd(std::string in);

#endif
