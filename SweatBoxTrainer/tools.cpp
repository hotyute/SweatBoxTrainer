#include "tools.h"

#include <windows.h>
#include <algorithm> 
#include <bitset>
#include <functional>
#include <cctype>
#include <locale>
#include <iostream>
#include <sstream>

std::vector<std::string>& split(const std::string& str, const std::string& delimiters, std::vector<std::string>& elems, int times) {
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	if (times != -1)
	{
		while (times > 0 && (std::string::npos != pos || std::string::npos != lastPos)) {
			// Found a token, add it to the vector.
			elems.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);

			--times;
		}
	}
	else
	{
		while (std::string::npos != pos || std::string::npos != lastPos) {
			// Found a token, add it to the vector.
			elems.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}
	return elems;
}


std::vector<std::string> split(const std::string& s, const std::string& delim, int times) {
	std::vector<std::string> elems;
	return split(s, delim, elems, times);
}

std::vector<std::string> split(const std::string& s, const std::string& delim) {
	return split(s, delim, -1);
}

char* s2ca1(const std::string& s) {
	char* res = new char[s.size() + 1];
	strncpy_s(res, s.size() + 1, s.c_str(), s.size() + 1);
	return res;
}

int random(int start, int end)
{
	if (end < start) {
		int temp = start;
		start = end;
		end = temp;
	}
	return start + (rand() % end);
}

long long doubleToRawBits(double x) {
	long long bits;
	memcpy(&bits, &x, sizeof bits);
	return bits;
}

Point2 getLocFromBearing(double latitude, double longitude, double distance, double bearing) {
	double R = 6378.14;

	// Degree to Radian
	double latitude1 = radians(latitude);
	double longitude1 = radians(longitude);
	double brng = radians(bearing);

	double latitude2 = asin(sin(latitude1) * cos(distance / R) + cos(latitude1) * sin(distance / R) * cos(brng));
	double longitude2 = longitude1 + atan2(sin(brng) * sin(distance / R) * cos(latitude1), cos(distance / R) - sin(latitude1) * sin(latitude2));

	// back to degrees
	latitude2 = degrees(latitude2);
	longitude2 = degrees(longitude2);

	// 8 decimal for Leafletand other system compatibility
	double lat2 = round_up(latitude2, 8);
	double long2 = round_up(longitude2, 8);

	return Point2(long2, lat2);
}

double get_distance(double speed_knots, double interval_ms) {
	return speed_knots * ((interval_ms / 1000) / 3600);
}

double radians(double degrees) {
	return (degrees * M_PI) / 180;
}

double degrees(double radians) {
	return (radians * 180) / M_PI;
}

double dist(double lat1, double lon1, double lat2, double lon2) {
	double dist, dlon = lon2 - lon1;
	lat1 *= M_PI / 180.0;
	lat2 *= M_PI / 180.0;
	dlon *= M_PI / 180.0;
	dist = (sin(lat1) * sin(lat2)) + (cos(lat1) * cos(lat2) * cos(dlon));
	if (dist > 1.0) dist = 1.0;
	dist = acos(dist) * 60 * 180 / M_PI;
	return dist;
}

double round_up(double value, int decimal_places) {
	const double multiplier = std::pow(10.0, decimal_places);
	return std::ceil(value * multiplier) / multiplier;
}

std::wstring s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
