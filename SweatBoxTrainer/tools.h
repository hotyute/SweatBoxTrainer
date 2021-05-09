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

double GetHeading(double lat1, double lat2, double lon1, double lon2);

double GetCTE(double current_lat, double current_lon, double dest_lat, double dest_lon, double track_angle);

double GetCTE2(Point2& p_from, Point2& p_to, double acf_lat, double acf_lon, double speed);

double get_roll(double start_roll, double end_roll, double total_ms, long long interval_ms);

double get_rot(double roll, double TAS, long long interval_ms);

double get_ros(double acceleration, long long interval_ms);

double get_distance(double speed, long long interval_ms);

double get_angle(double brgto, double brgfrom);

double get_angle_unsigned(double brgto, double brgfrom);

double radians(double degrees);

double degrees(double radians);

double dist(double lat1, double lon1, double lat2, double lon2);

double round_up(double value, int decimal_places);

std::wstring s2ws(const std::string& s);

std::string ws2s(const std::wstring& wstr);

double atodd(std::string in);

bool pnpoly(int nvert, int* vertx, int* verty, int testx, int testy);

void capitalize(std::string& str);

std::string ltrim(std::string s);

std::string rtrim(std::string s);

std::string trim(std::string s);

double hdg(double heading);

double get_bearing(double lat1, double long1, double lat2, double long2);

template<typename T>
int pushBack(std::vector<T>& v, T val)
{
	int retval = numbers.size();
	v.push_back(val);
	return retval;
}

#endif
