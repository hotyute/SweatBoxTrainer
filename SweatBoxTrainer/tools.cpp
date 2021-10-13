#include "tools.h"

#include <windows.h>
#include <algorithm> 
#include <bitset>
#include <functional>
#include <cctype>
#include <locale>
#include <iostream>
#include <sstream>

#include "constants.h"

const double EARTH_RADIUS_KM = 6378.14; //KM
const double EARTH_RADIUS_NM = 3437.670013352;
const int TRANSITION = 18000;

const int TURN_RATE_TAXI_MIN = 5;       // Degrees per second.
const int TURN_RATE_TAXI = 20;          // Degrees per second.
const int SPEED_MIN = 10;
const int SPEED_MAX = 20;

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

double NauticalMilesPerDegreeLon(double lat)
{
	return (M_PI / 180.0) * EARTH_RADIUS_NM * cos(radians(lat));
}

Point2 getLocFromBearing(double latitude, double longitude, double distancenm, double bearing) {

	double R = 6378.14;
	// Degree to Radian
	double latitude1 = radians(latitude);
	double longitude1 = radians(longitude);
	double brng = radians(bearing);

	double latitude2 = asin(sin(latitude1) * cos(distancenm / EARTH_RADIUS_NM) + cos(latitude1) * sin(distancenm / EARTH_RADIUS_NM) * cos(brng));
	double longitude2 = longitude1 + atan2(sin(brng) * sin(distancenm / EARTH_RADIUS_NM) * cos(latitude1), cos(distancenm / EARTH_RADIUS_NM) - sin(latitude1) * sin(latitude2));

	// back to degrees
	latitude2 = degrees(latitude2);
	longitude2 = degrees(longitude2);

	// 8 decimal for Leafletand other system compatibility
	double lat2 = round_up(latitude2, 8);
	double long2 = round_up(longitude2, 8);

	return Point2(long2, lat2);
}

double GetDistance(Point2* p1, Point2* p2)
{
	return GetDistance(p1->y_, p2->y_, p1->x_, p2->x_);
}

double GetDistance(double lat, double lon, Point2* p)
{
	return GetDistance(lat, p->y_, lon, p->x_);
}

double GetDistance(double lat1, double lat2, double lon1, double lon2)
{
	double lat11 = radians(lat1), lat22 = radians(lat2), lon11 = radians(lon1), lon22 = radians(lon2);
	double d;
	d = acos((sin(lat11) * sin(lat22)) + (cos(lat11) * cos(lat22) * cos(lon22 - lon11))) * EARTH_RADIUS_NM;
	return d;
}

long double distanceTo(long double lat1, long double long1,
	long double lat2, long double long2)
{
	// Convert the latitudes
	// and longitudes
	// from degree to radians.
	lat1 = radians(lat1);
	long1 = radians(long1);
	lat2 = radians(lat2);
	long2 = radians(long2);

	// Haversine Formula
	long double dlong = long2 - long1;
	long double dlat = lat2 - lat1;

	long double ans = pow(sin(dlat / 2), 2) +
		cos(lat1) * cos(lat2) *
		pow(sin(dlong / 2), 2);

	ans = 2 * asin(sqrt(ans));

	// Radius of Earth in
	// Kilometers, R = 6371
	// Use R = 3956 for miles
	long double R = 6371;

	// Calculate the result
	ans = ans * R;

	return ans;
}

double GetHeading(Point2* p1, Point2* p2)
{
	return GetHeading(p1->y_, p2->y_, p1->x_, p2->x_);
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
	cte = asin(sin(dist1 / EARTH_RADIUS_KM) * sin(bearing - track_angle)) * EARTH_RADIUS_KM;
	return cte;
}

double GetCTE2(Point2& p_from, Point2& p_to, double acf_lat, double acf_lon, double speed)
{
	double _lead_angle_limit = 45;//maximum intercept degrees
	double _lead_angle_gain = 1.5;//how quick it recovers from intercept angle
	double _proportion = 0.75;// where it will start on the course?

	double _wp_range = GetDistance(acf_lat, p_to.y_, acf_lon, p_to.x_);//wp_range
	double course = GetHeading(p_from.y_, p_to.y_, p_from.x_, p_to.x_);
	double brg = GetHeading(acf_lat, p_from.y_, acf_lon, p_from.x_);

	double xtrack_error_nm = sin(course - brg) * _wp_range;

	double factor = -0.0045 * speed + 1;

	double limit = _lead_angle_limit * factor;

	double _lead_angle = _wp_range > 0 ? degrees(atan2(xtrack_error_nm, (_wp_range * _proportion))) : 0;

	_lead_angle *= _lead_angle_gain * factor;

	double _xtrack_error = xtrack_error_nm * 6076.1155;//changing to feet

	_lead_angle = fmod(_lead_angle, limit);

	return _lead_angle;
}

double get_roll(double start_roll, double end_roll, double total_ms, long long interval_ms)
{
	return (end_roll - start_roll) * (interval_ms / total_ms);
}

double get_radius_of_turn(double angle, double radius)
{
	double result = 0.0;
	double loop_factor = angle / 90.0;
	while (loop_factor > 0)
	{
		double a = angle;
		if (loop_factor >= 1)
		{
			a = 90.0;
			angle -= 90.0;
		}
		else
		{
			a = angle;
		}
		result += tan(radians(a / 2.0)) * radius;
		--loop_factor;
	}
	return result;
}

double get_rot(double roll, double TAS, long long interval_ms) {
	double G = 1092.0;
	return (G * tan(radians(roll)) / TAS) * (interval_ms / 1000.0);
}

double get_rot(double roll, double TAS) {
	double G = 1092.0;
	return (G * tan(radians(roll)) / TAS);
}

double get_ros(double acceleration, long long interval_ms) {
	double G = 1092.0;
	return acceleration * (interval_ms / 1000.0);
}

double get_distance(double speed_knots, long long interval_ms) {
	return speed_knots * ((interval_ms / 1000.0) / 3600.0);
}

double get_angle(double a1, double a2)
{
	return min(fmod((a1 - a2 + 360), 360), fmod((a2 - a1 + 360), 360));
}

double get_angle_unsigned(double brgto, double brgfrom)
{
	return fmod(((brgto - brgfrom) + 540.0), 360) - 180.0;
}

double radians(double degs) {
	return (degs * M_PI) / 180.0;
}

double degrees(double rads) {
	return (rads * 180.0) / M_PI;
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

double dist2(double x1, double y1, double x2, double y2) {
	double dx = x2 - x1, dy = y2 - y1;
	return sqrt((dx * dx) + (dy * dy));
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

std::string ws2s(const std::wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), 0, 0, 0, 0);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), &strTo[0], size_needed, 0, 0);
	strTo.erase(strTo.size() - 1);
	return strTo;
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

bool intersects(Point2& p1, Point2& p2, Point2& c, double r)
{
	double width = p2.x_ - p1.x_;
	double height = p2.x_ - p1.x_;
	double x = c.x_ - p1.x_;
	double y = c.y_ - p1.y_;

	if (x > (width / 2 + r)) { return false; }
	if (y > (height / 2 + r)) { return false; }

	if (x <= (width / 2)) { return true; }
	if (y <= (height / 2)) { return true; }

	double cornerDistance_sq = ((x - width / 2) * (x - width / 2)) +
		((y - height / 2) * (y - height / 2));

	return (cornerDistance_sq <= (r * r));
}

double line_dist(Point2& l1, Point2& l2, Point2& p)
{
	double lat1 = l1.y_;
	double lon1 = l1.x_;
	double lat2 = l2.y_;
	double lon2 = l2.x_;
	double lat3 = p.y_;
	double lon3 = p.x_;

	double y = sin(lon3 - lon1) * cos(lat3);
	double x = cos(lat1) * sin(lat3) - sin(lat1) * cos(lat3) * cos(lat3 - lat1);
	double bearing1 = degrees(atan2(y, x));
	bearing1 = 360 - fmod((bearing1 + 360), 360);

	double y2 = sin(lon2 - lon1) * cos(lat2);
	double x2 = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lat2 - lat1);
	double bearing2 = degrees(atan2(y2, x2));
	bearing2 = 360 - fmod((bearing2 + 360), 360);

	double lat1Rads = radians(lat1);
	double lat3Rads = radians(lat3);
	double dLon = radians(lon3 - lon1);

	double distanceAC = acos(sin(lat1Rads) * sin(lat3Rads) + cos(lat1Rads) * cos(lat3Rads) * cos(dLon)) * EARTH_RADIUS_KM;
	double min_distance = fabs(asin(sin(distanceAC / EARTH_RADIUS_KM) * sin(radians(bearing1) - radians(bearing2))) * EARTH_RADIUS_KM);

	return min_distance;
}

double dis(double latA, double lonA, double latB, double lonB) {
	double R = 6371000;
	return acos(sin(latA) * sin(latB) + cos(latA) * cos(latB) * cos(lonB - lonA)) * R;
}

double bear(double latA, double lonA, double latB, double lonB) {
	// BEAR Finds the bearing from one lat / lon point to another.
	return atan2(sin(lonB - lonA) * cos(latB), cos(latA) * sin(latB) - sin(latA) * cos(latB) * cos(lonB - lonA));
}

double pointToLineDistance(Point2& l1, Point2& l2, Point2& p) {
	double lat1 = radians(l1.y_);
	double lat2 = radians(l2.y_);
	double lat3 = radians(p.y_);
	double lon1 = radians(l1.x_);
	double lon2 = radians(l2.x_);
	double lon3 = radians(p.x_);

	// Earth's radius in meters
	double R = 6371000;

	// Prerequisites for the formulas
	double bear12 = bear(lat1, lon1, lat2, lon2);
	double bear13 = bear(lat1, lon1, lat3, lon3);
	double dis13 = dis(lat1, lon1, lat3, lon3);

	// Is relative bearing obtuse?
	if (abs(bear13 - bear12) > (M_PI / 2))
		return dis13;

	// Find the cross-track distance.
	double dxt = asin(sin(dis13 / R) * sin(bear13 - bear12)) * R;

	// Is p4 beyond the arc?
	double dis12 = dis(lat1, lon1, lat2, lon2);
	double dis14 = acos(cos(dis13 / R) / cos(dxt / R)) * R;
	if (dis14 > dis12)
		return dis(lat2, lon2, lat3, lon3);
	return abs(dxt);
}

Point2* intersect(double $p1_lat, double $p1_lon, double $brng1, double $p2_lat, double $p2_lon, double $brng2) {
	double lat1 = radians($p1_lat), lon1 = radians($p1_lon);
	double lat2 = radians($p2_lat), lon2 = radians($p2_lon);
	double brng13 = radians($brng1);
	double brng23 = radians($brng2);
	double dLat = lat2 - lat1;
	double dLon = lon2 - lon1;

	double dist12 = 2 * asin(sqrt(sin(dLat / 2) * sin(dLat / 2) +
		cos(lat1) * cos(lat2) * sin(dLon / 2) * sin(dLon / 2)));
	if (dist12 == 0)
	{
		return nullptr;
	}

	// initial/final bearings between points
	double brngA = acos((sin(lat2) - sin(lat1) * cos(dist12)) /
		(sin(dist12) * cos(lat1)));
	if (isnan(brngA)) {
		brngA = 0;  // protect against rounding
	}
	double brngB = acos((sin(lat1) - sin(lat2) * cos(dist12)) /
		(sin(dist12) * cos(lat2)));

	double brng12, brng21;

	if (sin(lon2 - lon1) > 0)
	{
		brng12 = brngA;
		brng21 = 2 * M_PI - brngB;
	}
	else
	{
		brng12 = 2 * M_PI - brngA;
		brng21 = brngB;
	}

	double alpha1 = fmod((brng13 - brng12 + M_PI), (2 * M_PI)) - M_PI;  // angle 2-1-3
	double alpha2 = fmod((brng21 - brng23 + M_PI), (2 * M_PI)) - M_PI;  // angle 1-2-3

	if (sin(alpha1) == 0 && sin(alpha2) == 0) return nullptr;  // infinite intersections
	if (sin(alpha1) * sin(alpha2) < 0) return nullptr;       // ambiguous intersection

	double alpha3 = acos(-cos(alpha1) * cos(alpha2) +
		sin(alpha1) * sin(alpha2) * cos(dist12));
	double dist13 = atan2(sin(dist12) * sin(alpha1) * sin(alpha2), cos(alpha2) + cos(alpha1) * cos(alpha3));
	double lat3 = asin(sin(lat1) * cos(dist13) +
		cos(lat1) * sin(dist13) * cos(brng13));
	double dLon13 = atan2(sin(brng13) * sin(dist13) * cos(lat1), cos(dist13) - sin(lat1) * sin(lat3));
	double lon3 = lon1 + dLon13;

	lon3 = fmod((lon3 + 3 * M_PI), (2 * M_PI)) - M_PI;  // normalise to -180..+180º

	return new Point2(degrees(lon3), degrees(lat3));
}

bool isHeavyAngle(Point2* o, Point2* p, Point2* p2)
{
	double course = degrees(GetHeading(o->y_, p->y_, o->x_, p->x_));
	double locBrg = hdg(degrees(GetHeading(p->y_, p2->y_, p->x_, p2->x_)));
	return get_angle(locBrg, course) > 90;
}

double TurnRadius(double speed, double turnRate)
{
	double nmPerSec = speed / 3600.0;
	//double degPerSec = nmPerSec / NM_PER_DEG;
	double fullTurnTime = 360.0 / turnRate;
	//double circumference = degPerSec * fullTurnTime;
	double circumference = nmPerSec * fullTurnTime;
	return circumference / (2.0 * M_PI);
}

double HeadingDelta(double hdg1, double hdg2)
{
	double delta = abs(hdg2 - hdg1);
	if (delta > 180.0) {
		delta = 360.0 - delta;
	}
	return delta;
}

double GetDecelerationDistance(double initialSpeed, double finalSpeed, double decelRate)
{
	return ((initialSpeed * initialSpeed) - (finalSpeed * finalSpeed)) / (2.0 * decelRate * 3600.0) * DEG_PER_NM;
}

double CalcTaxiTurnRate(double turnAngle)
{
	double turnRate = TURN_RATE_TAXI;
	if (turnAngle <= 90.0) {
		double pct = turnAngle / 90.0;
		turnRate = TURN_RATE_TAXI_MIN + ((TURN_RATE_TAXI - TURN_RATE_TAXI_MIN) * pct);
	}
	return turnRate;
}

double CalcTaxiSpeed(double turnAngle, double maxSpeed)
{
	if (turnAngle < 1.0)
		return maxSpeed;

	double turnSpeed = maxSpeed;
	if (turnAngle > 45.0) {
		double pct = (turnAngle - 45.0) / (180.0 - 45.0);
		turnSpeed = SPEED_MIN + ((maxSpeed - SPEED_MIN) * (1.0 * pct));
	}
	return turnSpeed;
}


std::string FormatAltitude(std::string altitude)
{
	if (altitude.size() == 3)
	{
		if (altitude[0] != '0' && altitude[1] != '0')
		{
			while (altitude.size() < 5)
				altitude += "0";
		}
	}
	return altitude;
}

bool is_digits(const std::string& str)
{
	return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}
