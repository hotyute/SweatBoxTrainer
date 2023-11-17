#include "aircraft.h"

#include <boost/algorithm/string.hpp>

#include "tools.h"
#include "geoutils.h"

std::unordered_map<std::string, Aircraft*>AcfMap;

Aircraft::Aircraft() {
	aMutex = CreateMutex(NULL, FALSE, L"Aircraft Mutex");
	created = false;
	que_delete = false;
	altitude = 0;
	pitch = 0;
	latitude = 0;
	longitude = 0;
	heading = 0;
	roll = 0;
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	std::fill_n(last_time, sizeof(last_time) / sizeof(last_time[0]), now);
	std::fill_n(frequency, sizeof(frequency) / sizeof(frequency[0]), 99998);
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

double Aircraft::getAltitude() {
	return Aircraft::altitude;
}

void Aircraft::setAltitude(double val) {
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
	speed = getNextSpeed((double)interval);
	last_time[2] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}

void Aircraft::updateRoll()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[4];
	roll = getNextRoll(static_cast<double>(interval));
	last_time[4] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}

void Aircraft::updatePitch()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[5];
	pitch = getNextPitch(static_cast<double>(interval));
	last_time[5] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}

void Aircraft::updateHeading()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[1];
	heading = getNextHeading((double)interval);
	if ((heading == assignedValues.asgd_heading) && roll != 0)
	{
		assignedValues.asdg_roll = roll = 0;
	}
	last_time[1] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}

void Aircraft::updateMovement()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[0];
	double dist = get_distance(speed, interval);
	Point2 p = getLocFromBearing(latitude, longitude, dist, heading);
	if (p.x_ != longitude || p.y_ != latitude)
	{
		//other moved set flags here
	}
	latitude = p.y_;
	longitude = p.x_;
	last_time[0] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
}

void Aircraft::updateAltitude()
{
	long long now = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
	long long interval = now - last_time[3];
	if ((altitude == assignedValues.asdg_altitude) && pitch != 0)
	{
		assignedValues.asdg_pitch = pitch = 0;
	}
	altitude = getNextAltitude((double)interval);
	last_time[3] = boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds();
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

bool Aircraft::onGround()
{
	Airport* apt = getAirport();
	if (altitude <= 0 || (airport && altitude <= airport->elevation + 2))
		return true;
	return false;
}

double Aircraft::GetTrackTurnData()
{
	if (onGround())
	{
		if (ground_cur && ground_next)
		{
			double locBrg0 = ground_prev ? degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_))
				: degrees(GetHeading(latitude, ground_cur->y_, longitude, ground_cur->x_));
			double locBrg1 = degrees(GetHeading(ground_cur->y_, ground_next->y_, ground_cur->x_, ground_next->x_));

			double angle = get_angle(locBrg1, locBrg0);
			return CalcTaxiTurnRate(angle);
		}
	}
	return assignedValues.asdg_gnd_turn_rate;
}

double Aircraft::GetTrackSpeedData()
{
	if (onGround())
	{
		if (ground_cur && ground_next)
		{
			double locBrg0 = ground_prev ? degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_))
				: degrees(GetHeading(latitude, ground_cur->y_, longitude, ground_cur->x_));
			double locBrg1 = degrees(GetHeading(ground_cur->y_, ground_next->y_, ground_cur->x_, ground_next->x_));

			double angle = get_angle(locBrg1, locBrg0);

			return CalcTaxiSpeed(angle, defaultValues.speed);
		}
	}
	return speed;
}

bool Aircraft::OnTrack()
{
	if (onGround())
	{
		if (ground_cur && ground_prev)
		{
			double __unnamed000 = ground_cur->y_, __unnamed001 = ground_cur->x_;
			double course = degrees(GetHeading(latitude, __unnamed000, longitude, __unnamed001));
			double locBrg = degrees(GetHeading(*ground_prev, *ground_cur));//localizer heading
			double delta = get_angle_unsigned(locBrg, course);
			double delta2 = get_angle_unsigned(locBrg, heading);
			//printf("delta: [%f] [%f] [%f]\n", delta, course, locBrg);
			return (delta < 1 && delta > -1) && (delta2 < 1 && delta2 > -1);
		}
	}
	return false;
}
double SOME_SMALL_THRESHOLD = 5.0;

double Aircraft::getNextPoint()
{
	if (onGround())
	{
		if (ground_cur)
		{
			if (arrived(ground_next))
			{
				if (ground_cur)
				{
					std::cout << "[Arrived at : " << ground_cur->parent->name << " : " << ground_cur->index << "]" << std::endl;
					while (ground_route.size() > 0 && (ground_cur->parent->name != ground_route.front()))
						ground_route.erase(ground_route.begin());
				}
				pollRoute();
			}
			else
			{
				if (!ground_prev)
				{
					/*if (init_crse_p)
					{
						double h = hdg(degrees(GetHeading(init_crse_p->y_, ground_cur->y_, init_crse_p->x_, ground_cur->x_)));
						double f_heading = calculateGain(*ground_cur, *init_crse_p, h);
						assignedValues.asgd_heading = hdg(f_heading); //hdg(h - GetCTE2(*ground_prev, *ground_cur, latitude, longitude, speed));
					}
					else
					{*/
					double brng = get_bearing(latitude, longitude, ground_cur->y_, ground_cur->x_);

					// Calculate and store the turn angle for later use in the update cycle
					this->initialTurnAngle = fabs(heading - brng);
					if (this->initialTurnAngle > 180.0) {
						this->initialTurnAngle = 360.0 - this->initialTurnAngle;
					}

					// Check if the aircraft has completed the turn
					if (fabs(heading - get_bearing(latitude, longitude, ground_cur->y_, ground_cur->x_)) < SOME_SMALL_THRESHOLD) {
						if (assignedValues.asdg_speed != defaultValues.speed)
							assignedValues.asdg_speed = defaultValues.speed;
						this->initialTurnAngle = -1.0;  // Reset the turn angle
					}
					else {
						double speedForTurn = calcSpeedForInitTurn(this->initialTurnAngle);
						assignedValues.asdg_speed = speedForTurn;
					}

					if (assignedValues.asgd_heading != brng)
					{
						assignedValues.asgd_heading = brng;
						return brng;
					}
					//}
				}
				else
				{
					double h = hdg(degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_)));
					double f_heading = calculateGain(*ground_cur, *ground_prev, h);
					assignedValues.asgd_heading = hdg(f_heading); //hdg(h - GetCTE2(*ground_prev, *ground_cur, latitude, longitude, speed));
				}
			}
			if (!takeoff())
				checkRateReset(false);
		}
		if (!takeoff())
			checkPathHolds();
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

void Aircraft::checkRateReset(bool no_track)
{
	if (locked_rate)
	{//if not turning
		if (no_track || OnTrack())
		{
			if (assignedValues.asdg_gnd_turn_rate != DEFAULT_TURN_RATE)
				assignedValues.asdg_gnd_turn_rate = DEFAULT_TURN_RATE;
			if (assignedValues.asdg_speed != defaultValues.speed)
				assignedValues.asdg_speed = defaultValues.speed;
			locked_rate = false;
		}
	}
}

void Aircraft::pollRoute()
{
	Airport* airport_ptr = getAirport();

	if (airport_ptr)
	{
		if (onGround())
		{
			if (!ground_points.empty())
			{

				ground_cur ? ground_prev = ground_cur : ground_prev = nullptr;

				ground_cur = ground_points.front();

				auto next = ground_points.erase(ground_points.begin());

				next != ground_points.end() ? ground_next = *next : ground_next = nullptr;

				(next != ground_points.end() && ((next + 1) != ground_points.end()))
					? ground_next_next = *(next + 1) : ground_next_next = nullptr;

				if (!takeoff())
					set_taxing();
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

void Aircraft::refreshRoute()
{
	if (Airport* airport_ptr = getAirport())
	{
		if (onGround())
		{
			if (!ground_next && ground_cur && ground_points.size() >= 2)
			{
				auto next = (ground_points.begin() + 1);

				next != ground_points.end() ? ground_next = *next : ground_next = nullptr;

				(next != ground_points.end() && ((next + 1) != ground_points.end()))
					? ground_next_next = *(next + 1) : ground_next_next = nullptr;
			}
		}
	}
}

void Aircraft::prepareRoute()
{
	if (Airport* airport_ptr = getAirport())
	{
		if (onGround() && !ground_route.empty())
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
							p = path.angleTest(Point2(longitude, latitude), *p, *np);
							init_crse_p = path.getPrevPoint(p, path.getNextPoint(p, path.getClosest(next_path)));
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
							if (!taxiIntersect(*_end, *next_point))
							{
								ground_points.push_back(_end);
							}
							if (next_path->type == PATHTYPE::RUNWAY && (it + 2) == ground_route.end())
							{
								holds.push_back(next_point);
								runway_ctx = static_cast<Runway*>(next_path);
							}
							ground_points.push_back(next_point);
						}
					}
				}
			}
		}
	}
}

bool Aircraft::checkTooShort()
{
	if (ground_prev && ground_cur && ground_next && ground_next_next)
	{
		double turn_rate = onGround() ? assignedValues.asdg_gnd_turn_rate : get_rot(roll, speed, 1000);
		double next_speed = getNextSpeed(CALC_TIME);

		double brg0 = hdg(degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_)));
		double brg1 = hdg(degrees(GetHeading(ground_cur->y_, ground_next->y_, ground_cur->x_, ground_next->x_)));
		double brg2 = hdg(degrees(GetHeading(ground_next->y_, ground_next_next->y_, ground_next->x_, ground_next_next->x_)));

		double angle0 = get_angle(brg0, brg1);
		double angle1 = get_angle(brg1, brg2);

		double turn_radius0 = TurnRadius(speed, turn_rate);

		double distance0 = tan(radians(angle0 / 2.0)) * turn_radius0;
		double distance1 = tan(radians(angle1 / 2.0)) * turn_radius0;

		double dist_pt = GetDistance(ground_cur->y_, ground_next->y_, ground_cur->x_, ground_next->x_);

		if (dist_pt < (distance0 + distance1))
		{
			printf("Next path is too short!\n");
			return true;
		}
		return false;
	}
	return false;
}

bool Aircraft::arrived(Point2* p2)
{
	return arrived(ground_cur, p2);
}

bool Aircraft::arrived(Point2* p1, Point2* p2)
{
	if (!p1 || !p2)
		return false;

	/*if (checkTooShort())
	{
		if (doPointSkip())
		{

		}
	}*/

	if (!takeoff() && isTurnReady(p1, p2))
	{
		if (onGround())
		{
			assignedValues.asdg_gnd_turn_rate = GetTrackTurnData();
			assignedValues.asdg_speed = speed = GetTrackSpeedData();
			locked_rate = true;
			return true;
		}
	}

	return defaultTurnDistance();
}

void Aircraft::reset_path()
{
	ground_prev = nullptr;
	ground_cur = nullptr;
	ground_next = nullptr;
	ground_route.clear();
	ground_points.clear();
	holds.clear();
	checkRateReset(true);
	queue_takeoff = false;
	lineup = false;
	if (onGround() && !takeoff())
	{
		state = ACF_STATE::IDLE;
	}
}

void Aircraft::reset_holding()
{
	if (onGround() && holding())
	{
		if (!HoldingFor)
		{
			if (HoldingAt)
			{
				HoldingAt = nullptr;
				if (!HoldingDepart)
					set_taxing();
			}
			else if (!lineup && !queue_takeoff)
				set_taxing();
		}
	}
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
	if (speed != assignedValues.asdg_speed || (holding() && speed != 0))
	{
		double a_spd = ((onGround() && holding()) ? 0 : assignedValues.asdg_speed);
		double spd_delta = a_spd - speed;
		double acceleration = onGround() ? (spd_delta < 0 ? assignedValues.asdg_gnd_braking :
			(takeoff() ? perfValues.takeoff_accel : assignedValues.asdg_gnd_accel))
			: assignedValues.asdg_accel;
		double amount = get_per_second(acceleration, interval_ms);

		if (spd_delta != 0)
		{
			if (spd_delta < 0) {
				if ((speed + -amount) <= a_spd)
					next_speed = a_spd;
				else
					next_speed += -amount;
			}
			else if (spd_delta > 0)
			{
				if ((speed + amount) >= a_spd)
					next_speed = a_spd;
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
	double amount = onGround() ? (assignedValues.asdg_gnd_turn_rate * (interval_ms / 1000.0)) : get_rot(roll, speed, (long long)interval_ms);
	double new_hdg = get_angle_unsigned(hdg(assignedValues.asgd_heading), hdg(heading));
	double a_hdg = assignedValues.asgd_heading;
	if (speed != 0)
	{
		if (heading != a_hdg)
		{
			int turnOrientation = onGround() ? -1 : turnOri;
			if (turnOrientation == -1)
			{
				if (new_hdg > 0)
				{
					if (get_angle_unsigned(a_hdg, hdg((heading + amount))) <= 0)
						next_hdg = a_hdg;
					else
						next_hdg = hdg(heading + amount);
				}
				else if (new_hdg < 0)
				{
					if (get_angle_unsigned(a_hdg, hdg((heading + -amount))) >= 0)
						next_hdg = a_hdg;
					else
						next_hdg = hdg(heading + -amount);
				}
			}
			else if (turnOrientation == 0)//left turn
			{
				if (get_angle_unsigned(a_hdg, hdg((heading + -amount))) >= 0)
					next_hdg = a_hdg;
				else
					next_hdg = hdg(heading + -amount);
			}
			else // right turn
			{
				if (get_angle_unsigned(a_hdg, hdg((heading + amount))) <= 0)
					next_hdg = a_hdg;
				else
					next_hdg = hdg(heading + amount);
			}
		}
	}

	return next_hdg;
}

double Aircraft::getNextPitch(double interval_ms)
{
	double next_pitch = pitch;
	double amount = get_per_second(perfValues.pitch_rate, interval_ms);

	double a_pitch = assignedValues.asdg_pitch;

	double pitch_delta = a_pitch - pitch;

	if (pitch_delta < 0)
	{
		if ((next_pitch - amount) < a_pitch)
			next_pitch = a_pitch;
		else
			next_pitch -= amount;
	}
	else if (pitch_delta > 0)
	{
		if ((next_pitch + amount) > a_pitch)
			next_pitch = a_pitch;
		else
			next_pitch += amount;
	}

	return next_pitch;
}

double Aircraft::getNextRoll(double interval_ms)
{
	double next_roll = roll;
	double amount = get_per_second(perfValues.roll_rate, interval_ms);

	double a_roll = assignedValues.asdg_roll;

	double roll_delta = a_roll - roll;

	if (roll_delta < 0)
	{
		if ((next_roll - amount) < a_roll)
			next_roll = a_roll;
		else
			next_roll -= amount;
	}
	else if (roll_delta > 0)
	{
		if ((next_roll + amount) > a_roll)
			next_roll = a_roll;
		else
			next_roll += amount;
	}

	return next_roll;
}

double Aircraft::getNextAltitude(double interval_ms)
{
	double next_alt = altitude;
	double amount = get_per_minute(verticalSpeed, interval_ms);

	double a_alt = assignedValues.asdg_altitude;
	double alt_delta = a_alt - altitude;

	if (alt_delta < 0)
	{
		if ((next_alt - amount) < a_alt)
			next_alt = a_alt;
		else
			next_alt -= amount;
	}
	else if (alt_delta > 0)
	{
		if ((next_alt + amount) > a_alt)
			next_alt = a_alt;
		else
			next_alt += amount;
	}

	return next_alt;
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
	double turn_rate = onGround() ? GetTrackTurnData() : get_rot(roll, speed, 1000);
	double next_speed = getNextSpeed(CALC_TIME);
	double interval_dist = get_distance(next_speed, CALC_TIME);

	Point2 n = getLocFromBearing(latitude, longitude, interval_dist, heading);

	double locBrg0 = ground_prev ? hdg(degrees(GetHeading(ground_prev->y_, p->y_, ground_prev->x_, p->x_))) :
		hdg(degrees(GetHeading(latitude, p->y_, longitude, p->x_)));
	double locBrg1 = hdg(degrees(GetHeading(p->y_, p2->y_, p->x_, p2->x_)));

	double angle = get_angle(locBrg1, locBrg0);
	double time_sec = angle / turn_rate;
	double distance = next_speed * (time_sec / 3600.0);

	double dist_pt = GetDistance(latitude, p->y_, longitude, p->x_);

	double track_speed = GetTrackSpeedData();
	double turnRadius = TurnRadius(track_speed, turn_rate);
	double leadDistance = tan(radians(angle / 2.0)) * turnRadius;

	if ((dist_pt - leadDistance) <= GetDecelerationDistance(speed, track_speed, assignedValues.asdg_gnd_braking))
		assignedValues.asdg_speed = track_speed;

	/*if (dist_pt <= leadDistance)
	{
#ifdef _DEBUG
		printf("Turning at distance: %f, %f. angle: %f\n", dist_pt, leadDistance, angle);
#endif
		return true;
	}*/

	if (circularDistance(p, ((leadDistance * KNOTS_KM)) * 1000.0))
	{
#ifdef _DEBUG
		printf("Turning at distance: %f, %f. angle: %f\n", dist_pt, leadDistance, angle);
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

	Point2 n = getLocFromBearing(latitude, longitude, (interval_dist * KNOTS_KM), heading);

	double min_dist_from = (pointToLineDistance(*p, *p2, n) / 1000.0) / KNOTS_KM;

	//std::cout << min_dist_from << std::endl;

	if (min_dist_from <= distance)
		return true;

	return false;
}

bool Aircraft::defaultTurnDistance()
{
	return circularDistance(ground_cur, onGround() ? 2.2 : 7);
}

bool Aircraft::circularDistance(Point2* p, double distance_meters)
{

	double interval_dist = get_distance(speed, CALC_TIME);

	// we don't use the "future" point because we update the position just before calling "getNextPoint"
	Point2 n = getLocFromBearing(latitude, longitude, interval_dist, heading);

	Point2 v = getLocFromBearing(p->y_, p->x_, (distance_meters / 1000.0) / KNOTS_KM, 0);

	double num2 = (v.y_ - p->y_);
	double num3 = (v.x_ - p->x_);// prev 45.0 for boston

	double radius = sqrt((num3 * num3) + (num2 * num2));

	double dist_pt = GetDistance(n.y_, p->y_, n.x_, p->x_);

	if (inCircle2(Point2(longitude, latitude), n, *p, radius))
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

bool Aircraft::doPointSkip()
{
	if (ground_next && ground_next_next)
	{
		/*double turn_rate = onGround() ? assignedValues.asdg_gnd_turn_rate : get_rot(roll, speed, 1000);

		double next_speed = getNextSpeed(CALC_TIME);
		double interval_dist = get_distance(next_speed, CALC_TIME);

		Point2 n = getLocFromBearing(latitude, longitude, interval_dist, heading);

		double course = degrees(GetHeading(ground_next->y_, ground_next_next->y_, ground_next->y_, ground_next_next->x_));
		double angle = get_angle(course, heading);
		double time_sec = angle / turn_rate;
		double distance = next_speed * (time_sec / 3600.0);

		double dist_pt = GetDistance(latitude, ground_cur->y_, longitude, ground_cur->x_);

		double turnRadius = TurnRadius(speed, turn_rate);
		double leadDistance = tan(radians(angle / 2.0)) * turnRadius;

		printf("dist: %f, %f: angle(%f)\n", dist_pt, leadDistance, angle);

		if (dist_pt <= leadDistance)
		{
			return true;
		}
		return false;*/
		ground_points.erase(std::remove(ground_points.begin(), ground_points.end(), ground_next), ground_points.end());
		ground_next = nullptr;
		refreshRoute();
		return true;
	}
	return false;
}

void Aircraft::CheckFrameFlags()
{
	if (onGround())
	{
		if (OnTrack())
		{
			if (queue_takeoff && runway_ctx)
			{
				if (!lineup)
				{
					handle_takeoff_roll();
					queue_takeoff = false;
				}
				else
					set_holding();
			}
			else if (takeoff() && speed >= perfValues.v1)
			{
				handle_takeoff_rotate();
			}
		}
	}
}

void Aircraft::CollisionDetection()
{
	if (onGround())
	{
		if (HoldingFor)
		{
			Aircraft& other = *HoldingFor;
			double angle = get_angle(degrees(GetHeading(Point2(longitude, latitude), Point2(other.getLongitude(), other.getLatitude()))), heading);
			if (angle > 90.0 || GetDistance(Point2(other.getLongitude(), other.getLatitude()),
				Point2(longitude, latitude)) > (300 / KNOTS_FT))
			{
				HoldingFor = nullptr;
				if (!HoldingAt && !HoldingDepart)
					set_taxing();
			}
		}
		else if (AcfMap.size() > 0)
		{
			if (holding())
				return;
			Aircraft* hold_for = nullptr;
			double last_distance = 0;
			for (auto& it : AcfMap)
			{
				if (it.second != this)
				{
					Aircraft& other = *it.second;
					if (other.onGround() && (other.getAirport() == getAirport()))
					{
						double decel_distance0 = GetDecelerationDistance(speed, 0.0, assignedValues.asdg_gnd_braking);
						double cur_dist = GetDistance(other.GetNextLoc(), GetNextLoc());
						double dist = (300 / KNOTS_FT) + decel_distance0;
						if (cur_dist <= dist)
						{
							double angle = get_angle(degrees(GetHeading(GetNextLoc(), other.GetNextLoc())), heading);
							if ((last_distance == 0.0 || cur_dist < last_distance) && angle <= 90.0)
							{
								if ((ground_prev && (other.ground_prev && taxiIntersect(*ground_prev, *other.ground_prev)))
									|| (ground_cur && (other.ground_prev && taxiIntersect(*ground_cur, *other.ground_prev) ||
										other.ground_cur && taxiIntersect(*ground_cur, *other.ground_cur))))
								{
									hold_for = it.second;
									state = ACF_STATE::HOLDING;
									last_distance = cur_dist;
									printf("Holding For: %s\n", other.getCallSign().c_str());
								}
							}
						}
					}
				}
			}
			HoldingFor = hold_for;
		}
	}
}

void Aircraft::checkPathHolds()
{
	if (onGround())
	{
		if (HoldingAt || HoldingDepart)
		{

		}
		else
		{
			if (holding())
				return;
			if (holds.size() > 0)
			{
				auto it = holds.begin();
				while (it != holds.end())
				{
					Point2* p = *it;
					double decel_distance0 = GetDecelerationDistance(speed, 0.0, assignedValues.asdg_gnd_braking);
					double dist = (300 / KNOTS_FT) + decel_distance0;
					if (circularDistance(p, (dist * KNOTS_KM) * 1000.0))
					{
						state = ACF_STATE::HOLDING;
						it = holds.erase(it);
						//GetDistance(&Point2(longitude, latitude), p)
						if (p->parent)
						{
							printf("Holding at: %s\n", p->parent->name.c_str());
							if (runway_ctx && runway_ctx == p->parent)
							{
								HoldingDepart = p->parent;
							}
							HoldingAt = p->parent;
						}
						continue;
					}
					++it;
				}
			}
		}
	}
}

Point2 Aircraft::GetCurLoc()
{
	return Point2(longitude, latitude);
}

Point2 Aircraft::GetNextLoc()
{
	double next_heading = getNextHeading(CALC_TIME);
	double next_speed = getNextSpeed(CALC_TIME);
	double interval_dist = get_distance(next_speed, CALC_TIME);

	return getLocFromBearing(latitude, longitude, interval_dist, next_heading);
}

void Aircraft::HoldAt(std::string s)
{
	if (onGround())
	{
		Airport& airport = *getAirport();
		auto it3 = airport.all.find(s);
		if (it3 != airport.all.end())
		{
			TaxiPath& path = *it3->second;
			for (auto& p : ground_points)
			{
				if (path.intersect(*p))
				{
					holds.push_back(p);
					printf("Will hold at: %s\n", p->parent->name.c_str());
				}
			}
		}
	}
}

void Aircraft::handle_takeoff_roll()
{
	assignedValues.asdg_gnd_turn_rate = TURN_RATE_TAXI;
	locked_rate = false;
	assignedValues.asdg_speed = perfValues.v1;
	state = ACF_STATE::TAKEOFF;
}

void Aircraft::handle_takeoff_rotate()
{
	reset_path();
	assignedValues.asdg_gnd_turn_rate = 10;
	assignedValues.asdg_speed = perfValues.climb;
	assignedValues.asdg_altitude = perfValues.init_alt;
	pass_standard_pitch(perfValues.init_alt);
	state = ACF_STATE::AIRBORNE;
}

void Aircraft::pass_standard_pitch(double init_alt)
{
	if (init_alt > assignedValues.asdg_altitude)
		assignedValues.asdg_pitch = perfValues.max_pitch_up;
	else if (init_alt < assignedValues.asdg_altitude)
		assignedValues.asdg_pitch = perfValues.max_pitch_down;
}
