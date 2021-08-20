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
	speed = getNextSpeed(interval);
	last_time[2] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}

void Aircraft::updateHeading()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[1];
	heading = getNextHeading(interval);
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

double Aircraft::getNextPoint()
{
	if (onGround())
	{
		if (ground_cur)
		{
			if (arrived(ground_next))
			{
				std::cout << "[Arrived at : " << ground_next->parent->name << " : " << ground_next->index << "]" << std::endl;
				pollRoute();
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
					double h = hdg(degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_)));
					double f_heading = calculateGain(*ground_cur, *ground_prev, h);
					assignedValues.asgd_heading = hdg(f_heading); //hdg(h - GetCTE2(*ground_prev, *ground_cur, latitude, longitude, speed));
				}
			}
		}
	}
	else if (air_cur)
	{
		if (arrived(ground_next))
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
	return -1;
}

void Aircraft::pollRoute()
{
	Airport* airport_ptr = getAirport();

	if (airport_ptr)
	{
		if (onGround())
		{
			if (ground_points.size() > 1)
			{
				ground_cur ? ground_prev = ground_cur : ground_prev = nullptr;

				ground_cur = ground_points.front();

				auto next = ground_points.erase(ground_points.begin());

				next != ground_points.end() ? ground_next = *next : ground_next = nullptr;
			}
			else
			{
				reset_path();
			}
		}
		else
		{

		}
	}
}

void Aircraft::prepareRoute()
{
	Airport* airport_ptr = getAirport();

	if (airport_ptr)
	{
		if (onGround())
		{
			if (ground_route.size() > 0)
			{
				Airport& airport = *airport_ptr;

				for (auto it = ground_route.begin(); it != ground_route.end(); ++it)
				{
					auto it2 = airport.all.find(*it);
					if (it2 != airport.all.end())
					{
						TaxiPath& path = *it2->second;

						TaxiPath* next_path = ((it + 1) != ground_route.end()) ? airport.all.find(*(it + 1))->second : nullptr;

						if (ground_points.empty())
						{
							Point2* p = path.getClosestPoint(latitude, longitude);

							if (next_path)
							{
								Point2* np = path.getNextPoint(p, path.getClosest(next_path));
								p = path.angleTest(&Point2(longitude, latitude), p, np);
							}

							ground_points.push_back(p);
						}

						if (next_path)
						{
							Point2* last = ground_points.back();
							Point2* _end = path.getClosest(next_path);

							if (last && _end)
							{
								path.getPoints(last, _end, ground_points);
								Point2* next_point = next_path->getClosestPoint(_end->y_, _end->x_);
								if (GetDistance(_end, next_point) > (50 / KNOTS_FT))
								{
									ground_points.push_back(_end);
								}
								ground_points.push_back(next_point);
							}
						}
					}
				}

				/*auto it = ground_points.begin();
				while (it != ground_points.end())
				{
					Point2* point = *it;
					if (point)
					{
						if ((it + 1) != ground_points.end())
						{
							Point2* next_point = *(it + 1);
							if (next_point)
							{
								if (GetDistance(point, next_point) <= (50 / KNOTS_FT))
								{
									it = ground_points.erase(it);
									continue;
								}
							}
						}
					}
					++it;
				}*/
			}
		}
	}
}

bool Aircraft::arrived(Point2* p2)
{
	return arrived(ground_cur, p2);
}

bool Aircraft::arrived(Point2* p1, Point2* p2)
{
	if (!p1 || !p2)
		return false;

	return isTurnReady(p1, p2);
}

void Aircraft::reset_path()
{
	ground_prev = nullptr;
	ground_cur = nullptr;
	ground_next = nullptr;
	ground_route.clear();
	ground_points.clear();
}

double Aircraft::calculateGS(double __unnamed000, double __unnamed001, double gs_angle, double dest_altitude)//unamed 000 and 0001 dest latitude / longitude
{
	double num = (latitude - __unnamed000) * 60.0;
	double num2 = (longitude - __unnamed001) * NauticalMilesPerDegreeLon(latitude); // previously 45.0
	double num3 = sqrt(num2 * num2 + num * num) * 6076.1 * tan(0.052356020942408377) + (double)gs_angle;//GS ANGLE - 6076 is feet per NM
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

double Aircraft::getNextSpeed(double interval_ms)
{
	double next_speed = speed;
	if (speed != assignedValues.asdg_speed)
	{
		double acceleration = onGround() ? assignedValues.asdg_gnd_accel : assignedValues.asdg_accel;
		double amount = get_ros(acceleration, interval_ms);
		double spd_delta = assignedValues.asdg_speed - speed;

		if (spd_delta != 0)
		{
			if (spd_delta < 0) {
				if ((speed + -amount) <= assignedValues.asdg_speed)
					next_speed = assignedValues.asdg_speed;
				else
					next_speed += -amount;
			}
			else if (spd_delta > 0)
			{
				if ((speed + amount) >= assignedValues.asdg_speed)
					next_speed = assignedValues.asdg_speed;
				else
					next_speed += amount;
			}
		}
	}
	return next_speed;
}

double Aircraft::getNextHeading(double interval_ms)
{
	double next_hdg = heading;
	double amount = onGround() ? (assignedValues.asdg_gnd_turn_rate * (interval_ms / 1000.0)) : get_rot(roll, speed, interval_ms);
	double new_hdg = get_angle_unsigned(hdg(assignedValues.asgd_heading), hdg(heading));
	if (speed != 0)
	{
		/*if (ground_cur && ground_prev)
		{
			//double h = degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_));
			//next_hdg = hdg(h - GetCTE2(*ground_prev, *ground_cur, latitude, longitude, speed));
			double h = degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_));
			next_hdg = calculateGain(*ground_cur, *ground_prev, h);
		}
		else*/ if (heading != assignedValues.asgd_heading)
		{
			int turnOrientation = onGround() ? -1 : turnOri;
			if (turnOrientation == -1)
			{
				if (new_hdg > 0)
				{
					if (get_angle_unsigned(assignedValues.asgd_heading, hdg((heading + amount))) <= 0)
						next_hdg = assignedValues.asgd_heading;
					else
						next_hdg = hdg(heading + amount);
				}
				else if (new_hdg < 0)
				{
					if (get_angle_unsigned(assignedValues.asgd_heading, hdg((heading + -amount))) >= 0)
						next_hdg = assignedValues.asgd_heading;
					else
						next_hdg = hdg(heading + -amount);
				}
			}
			else if (turnOrientation == 0)//left turn
			{
				if (get_angle_unsigned(assignedValues.asgd_heading, hdg((heading + -amount))) >= 0)
					next_hdg = assignedValues.asgd_heading;
				else
					next_hdg = hdg(heading + -amount);
			}
			else // right turn
			{
				if (get_angle_unsigned(assignedValues.asgd_heading, hdg((heading + amount))) <= 0)
					next_hdg = assignedValues.asgd_heading;
				else
					next_hdg = hdg(heading + amount);
			}
		}
	}
	return next_hdg;
}

double Aircraft::calculateLoc(double __unnamed000, double __unnamed001, double __unnamed002, double default_hdg)
{
	double turn_rate = onGround() ? assignedValues.asdg_gnd_turn_rate : get_rot(roll, speed, 1000);
	double course = degrees(GetHeading(latitude, __unnamed000, longitude, __unnamed001));
	double d_lat = (latitude - __unnamed000) * 60.0;
	double d_lon = (longitude - __unnamed001) * NauticalMilesPerDegreeLon(latitude);
	double locBrg = __unnamed002;//localizer heading
	double num5 = sqrt(d_lon * d_lon + d_lat * d_lat) * sin((course - locBrg) * 0.017453277777777779);// * 0.017xxx is to radians
	double num6 = (double)speed * 0.033333333333333333 * 0.15915507752443828;
	double num7 = num6 - cos((heading - locBrg) * 0.017453277777777779) * num6;
	double num8 = abs(num5);
	if (num8 < (double)((150 / turn_rate) * 0.00013888888888888889))//adjust this number to a lower value to adjust precision
	{
		return (int)(num5 * 8.0 + locBrg + 720.0) % 360;
	}
	if (num7 < num8)// this is for checking how close we are to the bearing
	{
		return (double)default_hdg;//should be destination heading
	}
	return locBrg;
}

double Aircraft::calculateGain(Point2 cur, Point2 prev, double __unnamed002)
{
	double __unnamed000 = cur.y_, __unnamed001 = cur.x_;
	double course = degrees(GetHeading(latitude, __unnamed000, longitude, __unnamed001));
	double locBrg = __unnamed002;//localizer heading
	double delta = get_angle_unsigned(locBrg, course);

	double limit = 45;// *factor;
	double gain = delta < 0 ? limit : delta > 0 ? -limit : 0;

	double heading = calculateLoc(cur.y_, cur.x_, locBrg, hdg(locBrg + gain));

	if (heading == ((int)locBrg) && delta != 0)
	{
		/*#ifdef _DEBUG
			printf("on course: %f\n", delta);
		#endif*/
		heading = hdg(locBrg + -delta);
	}

	return heading;
}

/*Point2* intersection = intersect(latitude, longitude, hdg(locBrg + gain), prev.y_, prev.x_, locBrg);

	double heading = hdg(locBrg + gain);

	if (intersection)
	{
		if (isTurnReady(intersection, &cur))
		{
			//#ifdef _DEBUG
			//	std::cout << "on course: " << delta << std::endl;
			//#endif
			heading = hdg(locBrg + -delta);
		}
		delete intersection;
	}

	return heading;*/

double Aircraft::calculateTurnDistance(Point2* p, Point2* p2)
{
	double turn_rate = onGround() ? assignedValues.asdg_gnd_turn_rate : get_rot(roll, speed, 1000);
	double next_speed = getNextSpeed(CALC_TIME);
	double interval_dist = get_distance(next_speed, CALC_TIME);
	double next_heading = getNextHeading(CALC_TIME);

	double locBrg = hdg(degrees(GetHeading(p->y_, p2->y_, p->x_, p2->x_)));
	double angle = get_angle(locBrg, next_heading);
	double time_sec = angle / turn_rate;
	double distance = next_speed * (time_sec / 3600.0);

	return distance;
}


bool Aircraft::isTurnReady(Point2* p, Point2* p2)
{
	//TODO Possibly use "future" / next frame speed rather than current speed?
	double turn_rate = onGround() ? assignedValues.asdg_gnd_turn_rate : get_rot(roll, speed, 1000);
	double next_speed = getNextSpeed(CALC_TIME);
	double interval_dist = get_distance(next_speed, CALC_TIME);
	double next_heading = getNextHeading(CALC_TIME);

	Point2 n = getLocFromBearing(latitude, longitude, (interval_dist * KNOTS_KM), next_heading);

	double locBrg = hdg(degrees(GetHeading(p->y_, p2->y_, p->x_, p2->x_)));
	double angle = get_angle(locBrg, next_heading);
	double time_sec = angle / turn_rate;
	double distance = next_speed * (time_sec / 3600.0);

	//Point2 v = getLocFromBearing(p->y_, p->x_, (distance * KNOTS_KM), next_heading);

	//double remainder = fmod(GetDistance(n.y_, ground_cur->y_, n.x_, ground_cur->x_), distance);
	double dist_pt = GetDistance(n.y_, p->y_, n.x_, p->x_);

	if (dist_pt <= (distance * 1.2))
	{
	#ifdef _DEBUG
		printf("Turning at distance: %f\n", (distance * 1.2));
	#endif
		return true;
	}

	//calculate radius
	/*double num2 = (v.y_ - p->y_) / 60.0;
	double num3 = (v.x_ - p->x_) / NauticalMilesPerDegreeLon(p->y_);// prev 45.0 for boston

	double radius = sqrt((num3 * num3) + (num2 * num2));

	if (inCircle(Point2(longitude, latitude), n, *p, radius))
	{
		return true;
	}*/

	return false;
}

bool Aircraft::isTurnReady(Point2* p, Point2* p2, double distance)
{
	//TODO Possibly use "future" / next frame speed rather than current speed?
	double turn_rate = onGround() ? assignedValues.asdg_gnd_turn_rate : get_rot(roll, speed, 1000);
	double next_speed = getNextSpeed(CALC_TIME);
	double interval_dist = get_distance(next_speed, CALC_TIME);
	double next_heading = getNextHeading(CALC_TIME);

	Point2 n = getLocFromBearing(latitude, longitude, (interval_dist * KNOTS_KM), next_heading);

	double min_dist_from = (pointToLineDistance(*p, *p2, n) / 1000.0) / KNOTS_KM;

	//std::cout << min_dist_from << std::endl;

	if (min_dist_from <= distance)
		return true;

	return false;
}

bool Aircraft::defaultTurnDistance()
{
	return defaultTurnDistance(ground_cur);
}

bool Aircraft::defaultTurnDistance(Point2* p1)
{
	int radius_m = onGround() ? 2 : 20;

	double interval_dist = get_distance(speed, CALC_TIME);
	Point2 n = getLocFromBearing(latitude, longitude, (interval_dist * KNOTS_KM), heading);

	Point2 v = getLocFromBearing(p1->y_, p1->x_, (radius_m / 1000.0), 0);

	double num2 = (v.y_ - p1->y_) / 60.0;
	double num3 = (v.x_ - p1->x_) / NauticalMilesPerDegreeLon(p1->y_);// prev 45.0 for boston

	double radius = sqrt(num3 * num3 + num2 * num2);

	double dist_pt = GetDistance(n.y_, p1->y_, n.x_, p1->x_);

	/*if (inCircle(Point2(longitude, latitude), n, *p1, radius))
	{
		return true;
	}*/

	if (dist_pt <= ((radius_m / 1000.0) / KNOTS_KM))
		return true;

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

Point2* Aircraft::getFuturePoint(TaxiPath* cur_path, TaxiPath* next_path, Point2* taxiway_end)
{
	Point2* p2 = nullptr;
	if (ground_cur && taxiway_end)
	{
		//Point2** p = checkEarlyTurn();
		if (cur_path && (ground_cur != taxiway_end))
		{
			p2 = cur_path->getNextPoint(ground_cur, taxiway_end);
			std::cout << "Next Point: " << GetDistance(ground_cur->y_, p2->y_, ground_cur->x_, p2->x_) << std::endl;
		}
		else if (next_path)
		{
			//p2 = cur_path->getNextPoint(ground_cur, cur_path->getClosest(next_path));
			p2 = next_path->getClosestPoint(taxiway_end->y_, taxiway_end->x_);
			std::cout << "Next Path: " << GetDistance(ground_cur->y_, p2->y_, ground_cur->x_, p2->x_) << std::endl;
		}
	}

	return p2;
}

/*Point2** Aircraft::checkEarlyTurn()
{
	Point2* p2[2] = { nullptr, nullptr };
	if (next_path)
	{
		if (taxiway_end)
		{
			Point2* n = next_path->getClosestPoint(taxiway_end->y_, taxiway_end->x_);
			if (next_next_path)
			{
				Point2* e = next_path->getClosest(next_next_path);
				Point2* t = next_path->getNextPoint(n, e);

				double d1 = calculateTurnDistance(n, t);

				if (isTurnReady(n, t, d1))
				{
					std::cout << "Turning Early" << std::endl;
					p2[0] = n;
					p2[1] = t;
				}
			}
		}
	}
	return p2;
}*/
