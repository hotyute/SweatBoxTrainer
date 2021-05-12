#include "aircraft.h"

#include <boost/algorithm/string.hpp>

#include "tools.h"

std::unordered_map<std::string, Aircraft*>AcfMap;

Aircraft::Aircraft() {
	Aircraft::aMutex = CreateMutex(NULL, FALSE, L"Aircraft Mutex");
	Aircraft::created = false;
	Aircraft::que_delete = false;
	Aircraft::altitude = 0;
	Aircraft::pitch = 0;
	Aircraft::latitude = 0;
	Aircraft::longitude = 0;
	Aircraft::heading = 0;
	Aircraft::roll = 0;
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	for (int i = 0; i < 4; ++i)
	{
		last_time[0] = now;
	}
}

Aircraft::~Aircraft()
{

}

int Aircraft::getIndex() {
	return Aircraft::index;
}

void Aircraft::setIndex(int value) {
	Aircraft::index = value;
}

std::string Aircraft::getAcfTitle() {
	return Aircraft::acfTitle;
}

void Aircraft::setAcfTitle(std::string value) {
	Aircraft::acfTitle = value;
}

double Aircraft::getLatitude() {
	return Aircraft::latitude;
}

void Aircraft::setLatitude(double value) {
	Aircraft::latitude = value;
}

double Aircraft::getLongitude() {
	return Aircraft::longitude;
}

void Aircraft::setLongitude(double value) {
	Aircraft::longitude = value;
}

int Aircraft::getAltitude() {
	return Aircraft::altitude;
}

void Aircraft::setAltitude(int val) {
	Aircraft::altitude = val;
}

double Aircraft::getSpeed() {
	return Aircraft::speed;
}

void Aircraft::setSpeed(double value) {
	Aircraft::speed = value;
}

double Aircraft::getHeading() {
	return Aircraft::heading;
}

void Aircraft::setHeading(double value) {
	Aircraft::heading = value;
}

double Aircraft::getPitch() {
	return Aircraft::pitch;
}

void Aircraft::setPitch(double val) {
	Aircraft::pitch = val;
}

double Aircraft::getRoll() {
	return Aircraft::roll;
}

void Aircraft::setRoll(double val) {
	Aircraft::roll = val;
}

HANDLE Aircraft::getMutex() {
	return Aircraft::aMutex;
}

void Aircraft::lock() {
	WaitForSingleObject(Aircraft::aMutex, INFINITE);
}

void Aircraft::unlock() {
	ReleaseMutex(Aircraft::aMutex);
}

Aircraft* getAircraftByIndex(int index) {
	return NULL;
}

//void Aircraft::setUser1(User *value) {
//Aircraft::acfUser = value;
//}

bool Aircraft::isHeavy() {
	return Aircraft::heavy;
}

void Aircraft::setHeavy(bool value) {
	Aircraft::heavy = value;
}

std::string Aircraft::getSquawkCode() {
	return Aircraft::transponder;
}

void Aircraft::setSquawkCode(std::string value) {
	Aircraft::transponder = value;
}

int Aircraft::getMode() {
	return Aircraft::mode;
}

void Aircraft::setMode(int mode) {
	Aircraft::mode = mode;
}

void Aircraft::updateSpeed()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[2];

	if (speed != assignedValues.asdg_speed) {
		double acceleration = onGround() ? assignedValues.asdg_gnd_accel : assignedValues.asdg_accel;
		double amount = get_ros(acceleration, interval);
		double spd_delta = assignedValues.asdg_speed - speed;

		if (spd_delta != 0)
		{
			if (spd_delta < 0) {
				if ((speed + -amount) <= assignedValues.asdg_speed)
					speed = assignedValues.asdg_speed;
				else
					speed += -amount;
			}
			else if (spd_delta > 0)
			{
				if ((speed + amount) >= assignedValues.asdg_speed)
					speed = assignedValues.asdg_speed;
				else
					speed += amount;
			}
		}
	}

	last_time[2] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}

void Aircraft::updateHeading()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[1];
	double amount = onGround() ? (assignedValues.asdg_gnd_turn_rate * (interval / 1000.0)) : get_rot(roll, speed, interval);
	double new_hdg = get_angle_unsigned(hdg(assignedValues.asgd_heading), hdg(heading));
	if (speed != 0) {
		if (heading != assignedValues.asgd_heading) {
			int turnOrientation = onGround() ? -1 : turnOri;
			if (turnOrientation == -1)
			{
				if (new_hdg > 0)
				{
					if (get_angle_unsigned(assignedValues.asgd_heading, hdg((heading + amount))) <= 0)
						heading = assignedValues.asgd_heading;
					else
						heading = hdg(heading + amount);
				}
				else if (new_hdg < 0)
				{
					if (get_angle_unsigned(assignedValues.asgd_heading, hdg((heading + -amount))) >= 0)
						heading = assignedValues.asgd_heading;
					else
						heading = hdg(heading + -amount);
				}
			}
			else if (turnOrientation == 0)//left turn
			{
				if (get_angle_unsigned(assignedValues.asgd_heading, hdg((heading + -amount))) >= 0)
					heading = assignedValues.asgd_heading;
				else
					heading = hdg(heading + -amount);
			}
			else // right turn
			{
				if (get_angle_unsigned(assignedValues.asgd_heading, hdg((heading + amount))) <= 0)
					heading = assignedValues.asgd_heading;
				else
					heading = hdg(heading + amount);
			}
		}
	}
	last_time[1] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}



void Aircraft::updateMovement()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[0];
	double dist = get_distance(speed, interval);
	Point2 p = getLocFromBearing(latitude, longitude, (dist * KNOTS_KM), heading);
	if (p.x_ != longitude || p.y_ != latitude)
	{
		//aircraft moved set flags here
	}
	latitude = p.y_;
	longitude = p.x_;
	last_time[0] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}

Airport* Aircraft::getAirport()
{
	if (!empty(apt_icao))
	{
		if (!airport || !boost::iequals(airport->icao, apt_icao))
		{
			auto it = airports.find(apt_icao);
			if (it != airports.end()) {
				airport = it->second;
				return airport;
			}
		}
		return airport;
	}
	return nullptr;
}

bool Aircraft::onGround() {
	Airport* apt = getAirport();
	if (altitude <= 0 || (airport && altitude <= airport->elevation + 2))
		return true;
	return false;
}

double Aircraft::getNextHeading()
{
	if (onGround()) {
		if (ground_cur)
		{
			if (arrived())
			{
				std::cout << "hi" << std::endl;
				ground_prev = ground_cur;
				if (ground_cur == taxiway_end)
				{
					std::cout << "reached" << std::endl;
					cur_path = nullptr;
					taxiway_end = nullptr;
					ground_prev = nullptr;
					ground_route.erase(ground_route.begin());
				}
				ground_cur = nullptr;

				processRoute();
			}
			else
			{
				if (!ground_prev)
				{
					double brng = get_bearing(latitude, longitude, ground_cur->y_, ground_cur->x_);
					if (assignedValues.asgd_heading != brng)
					{
						assignedValues.asgd_heading = brng;
						return brng;
					}
				}
				else
				{
					double h = degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_));
					double heading = calculateLoc(ground_cur->y_, ground_cur->x_, h);
					assignedValues.asgd_heading = hdg(heading);//hdg(heading - GetCTE2(*ground_prev, *ground_cur, latitude, longitude, speed));
				}
			}
		}
	}
	else
	{
		if (air_cur)
		{
			if (arrived())
			{
				air_prev = air_cur;
				air_cur = nullptr;
			}
			else
			{
				if (!air_prev)
				{
					double brng = get_bearing(latitude, longitude, air_cur->y_, air_cur->x_);
					if (assignedValues.asgd_heading != brng)
					{
						assignedValues.asgd_heading = brng;
						return brng;
					}
				}
			}
		}
	}
	return -1;
}

void Aircraft::processRoute()
{
	Airport* airport_ptr = getAirport();

	if (airport_ptr)
	{
		if (onGround())
		{
			if (ground_route.size() > 0)
			{
				Airport& airport = *airport_ptr;

				if (!ground_cur && !ground_prev)// initial point
				{
					auto it = airport.all.find(ground_route.front());
					if (it != airport.all.end())
					{
						TaxiPath& tp = *it->second;
						cur_path = it->second;

						auto it2 = ++it;
						if (it2 != airport.all.end())
							next_path = it2->second;
						else next_path = nullptr;

						ground_cur = tp.getClosestPoint(latitude, longitude);

						future_point = getFuturePoint();
					}
				}
				else if (!ground_cur && ground_prev)// its arrived at the point and needs new point
				{
					TaxiPath* tp_ptr = cur_path;
					if (!tp_ptr)
					{
						auto it = airport.all.find(ground_route.front());
						if (it != airport.all.end())
						{
							tp_ptr = it->second;
						}
					}

					if (tp_ptr)
					{
						TaxiPath& tp = *tp_ptr;
						auto next = ground_route.begin() + 1;
						if (next != ground_route.end())
						{
							if (!taxiway_end)
							{
								auto it = airport.all.find(*next);
								if (it != airport.all.end())
								{
									taxiway_end = tp.getClosest(it->second);
								}
							}

							if (taxiway_end) 
							{
								ground_cur = tp.getNextPoint(ground_prev, taxiway_end);

								future_point = getFuturePoint();
							}
						}
					}
				}
			}
		}
	}
}

bool Aircraft::arrived()
{
	if (!ground_cur)
		return false;

	Point2* p2 = future_point;
	if (ground_cur && p2)
	{
		return calculateTurnDistance(ground_cur, p2);
	}
	return defaultTurnDistance();
}

double Aircraft::calculateGS(double __unnamed000, double __unnamed001, double gs_angle, double dest_altitude)//unamed 000 and 0001 dest latitude / longitude
{
	double num = (latitude - __unnamed000) * 60.0;
	double num2 = (longitude - __unnamed001) * NauticalMilesPerDegreeLon(latitude); // previously 45.0
	double num3 = sqrt(num2 * num2 + num * num) * 6076.1 * tan(0.052356020942408377) + (double)gs_angle;//GS ANGLE
	double num4 = dest_altitude;//Destination altitude
	double num5 = ((!(num3 < num4)) ? num4 : num3);
	if (num5 < (double)altitude)
	{
		num4 = dest_altitude;//Destination Altitude
		if (num3 < num4)
		{
			return num3;
		}
		return num4;
	}
	return altitude;
}

double Aircraft::calculateLoc(double __unnamed000, double __unnamed001, int __unnamed002)
{
	double course = degrees(GetHeading(latitude, __unnamed000, longitude, __unnamed001));
	double d_lat = (latitude - __unnamed000) * 60.0;
	double d_lon = (longitude - __unnamed001) * NauticalMilesPerDegreeLon(latitude);
	double locBrg = __unnamed002;//localizer heading
	double num5 = sqrt(d_lon * d_lon + d_lat * d_lat) * sin((course - locBrg) * 0.017453277777777779);// * 0.017xxx is to radians
	double num6 = (double)speed * 0.033333333333333333 * 0.15915507752443828;
	double num7 = num6 - cos((heading - locBrg) * 0.017453277777777779) * num6;
	double num8 = abs(num5);
	if (num8 < (double)speed * 0.0013888888888888889)
	{
		return (int)(num5 * 8.0 + locBrg + 720.0) % 360;
	}
	if (num7 < num8)// this is for checking how close we are to the bearing
	{
		//return *(double*)((byte*)P_0 + 40);//destination heading
	}
	return locBrg;
}

bool Aircraft::calculateTurnDistance(Point2* p, Point2* p2)
{

	//TODO POssibly use "future" / next frame speed rather than current speed?
	double amount = onGround() ? assignedValues.asdg_gnd_turn_rate : get_rot(roll, speed, 1000);

	double course = hdg(degrees(GetHeading(latitude, ground_cur->y_, longitude, ground_cur->x_)));
	double locBrg = hdg(degrees(GetHeading(ground_cur->y_, p2->y_, ground_cur->x_, p2->x_)));

	double angle = get_angle(locBrg, course);

	double time_sec = angle / amount;

	double distance = speed * (time_sec / 3600.0);

	double cur_dist = GetDistance(latitude, ground_cur->y_, longitude, ground_cur->x_);

	if (GetDistance(latitude, ground_cur->y_, longitude, ground_cur->x_) <= distance)
		return true;

	return false;
}

bool Aircraft::defaultTurnDistance()
{
	int radius_m = onGround() ? 16 : 50;

	double interval_dist = get_distance(speed, CALC_TIME);
	Point2 n = getLocFromBearing(latitude, longitude, (interval_dist * KNOTS_KM), heading);

	Point2 v = getLocFromBearing(ground_cur->y_, ground_cur->x_, (radius_m / 1000.0), 0);

	double num2 = (v.y_ - ground_cur->y_) / 60.0;
	double num3 = (v.x_ - ground_cur->x_) / NauticalMilesPerDegreeLon(ground_cur->y_);// prev 45.0 for boston

	double radius = sqrt(num3 * num3 + num2 * num2);

	if (inCircle(Point2(longitude, latitude), n, *ground_cur, radius))
	{
		return true;
	}
	return false;
}

FlightPlan::FlightPlan()
{
	FlightPlan::squawkCode = "0000";
	FlightPlan::departure = "";
	FlightPlan::arrival = "";
	FlightPlan::alternate = "";
	FlightPlan::squawkCode = "";
	FlightPlan::acType = "";
	FlightPlan::scratchPad = "";
	FlightPlan::cruise = "";
	FlightPlan::route = "";
	FlightPlan::remarks = "";
}

void FlightPlan::updateFlightPlan(char* depart, char* arrive)
{
	FlightPlan::squawkCode = "0000";
	FlightPlan::departure = depart;
	FlightPlan::arrival = arrive;
	FlightPlan::cycle++;
}

Point2* Aircraft::getFuturePoint()
{
	Point2* p2 = nullptr;
	if (cur_path)
	{
		if (taxiway_end)
		{
			Point2* n = cur_path->getNextPoint(ground_cur, taxiway_end);
			if (n != taxiway_end)
				p2 = n;
		}
	}

	if (!p2 && next_path)
	{
		p2 = next_path->getClosestPoint(latitude, longitude);
	}

	return p2;
}
