#include "tools.h"

#include <windows.h>
#include <algorithm> 
#include <bitset>
#include <functional>
#include <cctype>
#include <locale>
#include <iostream>
#include <sstream>

const double R_EARTH = 6378.14;

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

double GetDistance(double lat1, double lat2, double lon1, double lon2)
{
	double d;
	d = acos((sin(lat1) * sin(lat2)) + (cos(lat1) * cos(lat2) * cos(lon2 - lon1))) * R_EARTH;
	return d;
}

double GetHeading(double lat1, double lat2, double lon1, double lon2)
{
	double lat11 = radians(lat1), lat22 = radians(lat2), lon11 = radians(lon1), lon22 = radians(lon2);
	double angle;
	angle = atan2(sin(lon22 - lon11) * cos(lat22), cos(lat11) * sin(lat22) - sin(lat11) * cos(lat22) * cos(lon22 - lon11));
	return (angle);
}

//cross track error
double GetCTE(double current_lat, double current_lon, double dest_lat, double dest_lon, double track_angle)
{
	double cte, dist1, bearing;

	dist1 = GetDistance(radians(current_lat), radians(dest_lat), radians(current_lon), radians(dest_lon));
	bearing = GetHeading(current_lat, dest_lat, current_lon, dest_lon);
	cte = asin(sin(dist1 / R_EARTH) * sin(bearing - track_angle)) * R_EARTH;
	return cte;
}

double GetCTE2(Point2& p_from, Point2& p_to, double acf_lat, double acf_lon, double speed)
{
	double _lead_angle_limit = 90;//maximum intercept degrees
	double _lead_angle_gain = 100;//how quick it recovers from intercept angle
	double _proportion = 0.1;// where it will start on the course?

	double _wp_range = GetDistance(radians(acf_lat), radians(p_to.y_), radians(acf_lon), radians(p_to.x_));//wp_range
	double course = GetHeading(p_from.y_, p_to.y_, p_from.x_, p_to.x_);
	double brg = GetHeading(acf_lat, p_from.y_, acf_lon, p_from.x_);

	double xtrack_error_nm = sin(course - brg) * _wp_range;

	double factor = -0.0045 * speed + 1;

	double limit = _lead_angle_limit * factor;

	double _lead_angle = 0;
	if (_wp_range > 0) {
		_lead_angle = degrees(atan2(xtrack_error_nm, (_wp_range * _proportion)));
	}
	else
		_lead_angle = 0;

	_lead_angle *= _lead_angle_gain * factor;

	double _xtrack_error = xtrack_error_nm * 6076.1155;//changing to km??

	_lead_angle = fmod(_lead_angle, limit);

	return _lead_angle;
}

double get_roll(double start_roll, double end_roll, double total_ms, long long interval_ms)
{
	return (end_roll - start_roll) * (interval_ms / total_ms);
}

double get_rot(double roll, double TAS, long long interval_ms) {
	double G = 1092.0;
	return (G * tan(radians(roll)) / TAS) * (interval_ms / 1000.0);
}

double get_ros(double acceleration, long long interval_ms) {
	double G = 1092.0;
	return acceleration * (interval_ms / 1000.0);
}

double get_distance(double speed_knots, long long interval_ms) {
	return speed_knots * ((interval_ms / 1000.0) / 3600.0);
}

double get_angle(double brgto, double brgfrom)
{
	return fmod((fmod((brgto - brgfrom), 360) + 540.0), 360) - 180.0;
}

double get_angle_unsigned(double brgto, double brgfrom)
{
	return fmod(((brgto - brgfrom) + 540.0), 360) - 180.0;
}

double radians(double degrees) {
	return (degrees * M_PI) / 180.0;
}

double degrees(double radians) {
	return (radians * 180.0) / M_PI;
}

double dist(double lat1, double lon1, double lat2, double lon2) {
	double dist, dlon = lon2 - lon1;
	lat1 *= M_PI / 180.0;
	lat2 *= M_PI / 180.0;
	dlon *= M_PI / 180.0;
	dist = (sin(lat1) * sin(lat2)) + (cos(lat1) * cos(lat2) * cos(dlon));
	if (dist > 1.0) dist = 1.0;
	dist = acos(dist) * 60.0 * 180.0 / M_PI;
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

double atodd(std::string in) {
	double d;
	size_t read = 0;
	d = std::stod(in, &read);
	if (in.size() != read || isnan(d))
		throw (0);

	return d;
}

bool pnpoly(int nvert, int* vertx, int* verty, int testx, int testy) {
	bool c = false;
	int i, j;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i])) {
			c = !c;
		}
	}
	return c;
}

void capitalize(std::string& str)
{
	for (auto& x : str)
		x = toupper(x);
}

// trim from start
std::string ltrim(std::string s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
	return s;
}

// trim from end
std::string rtrim(std::string s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c); }).base(), s.end());
	return s;
}

// trim from both ends
std::string trim(std::string s) {
	return ltrim(rtrim(s));
}

double hdg(double heading)
{
	double hdg = fmod(heading, 360);
	if (hdg < 0)
		hdg += 360.0;
	return hdg;
}

double get_bearing(double lat1, double long1, double lat2, double long2)
{
	double dLon = (long2 - long1);
	double x = cos(radians(lat2)) * sin(radians(dLon));
	double y = cos(radians(lat1)) * sin(radians(lat2)) - sin(radians(lat1))
		* cos(radians(lat2)) * cos(radians(dLon));
	double brng = atan2(x, y);
	brng = degrees(brng);

	return hdg(brng);
}

