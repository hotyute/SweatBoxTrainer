#include "SweatBoxTrainer/aircraft/RouteManager.h"

#include <algorithm>
#include <iostream>

#include "SweatBoxTrainer/tools.h"
#include "SweatBoxTrainer/geoutils.h"
#include "SweatBoxTrainer/constants.h"
#include "SweatBoxTrainer/aircraft/aircraft.h"
#include "SweatBoxTrainer/guidialogue.h"

namespace {
	bool pointOnPath(Point2* point, TaxiPath* path)
	{
		return point && path && point->parent &&
			(point->parent == path || point->parent->name == path->name);
	}
}

void RouteManager::prepareRoute(Airport* airport, const AircraftState& state)
{
	if (airport && !ground_route.empty())
	{
		for (auto it = ground_route.begin(); it != ground_route.end(); ++it)
		{
			auto it2 = airport->all.find(*it);
			if (it2 != airport->all.end())
			{
				TaxiPath& path = *it2->second;
				TaxiPath* next_path = ((it + 1) != ground_route.end()) ? airport->all.find(*(it + 1))->second : nullptr;

				if (ground_points.empty())
				{
					Point2* p = path.getClosestPoint(state.latitude, state.longitude);
					if (next_path)
					{
						Point2* np = path.getNextPoint(p, path.getClosest(next_path));
						p = path.angleTest(Point2(state.longitude, state.latitude), *p, *np);
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

bool RouteManager::pollRoute()
{
	if (!ground_points.empty())
	{
		ground_cur ? ground_prev = ground_cur : ground_prev = nullptr;
		ground_cur = ground_points.front();

		auto next = ground_points.erase(ground_points.begin());
		next != ground_points.end() ? ground_next = *next : ground_next = nullptr;

		(next != ground_points.end() && ((next + 1) != ground_points.end()))
			? ground_next_next = *(next + 1) : ground_next_next = nullptr;
		return true;
	}
	// Path finished, reset pointers
	ground_prev = ground_cur = ground_next = ground_next_next = nullptr;
	return false;
}

void RouteManager::resetPath(AssignedValues& assigned, DefaultValues& defaults)
{
	ground_prev = nullptr;
	ground_cur = nullptr;
	ground_next = nullptr;
	ground_route.clear();
	ground_points.clear();
	holds.clear();
	checkRateReset(assigned, defaults, true);
	queue_takeoff = false;
	lineup = false;
}

void RouteManager::resetHolding()
{
	HoldingDepart = nullptr;
	HoldingAt = nullptr;
	HoldingFor = nullptr;
}

void RouteManager::resetContext()
{
	runway_ctx = nullptr;
}

void RouteManager::HoldAt(Airport* airport, std::string name)
{
	if (airport) {
		auto it3 = airport->all.find(name);
		if (it3 != airport->all.end())
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

bool RouteManager::prepareRunwayDeparturePath(Runway* runway)
{
	if (!runway || !ground_cur)
		return false;

	Point2* runway_entry = nullptr;
	if (pointOnPath(ground_cur, runway))
	{
		runway_entry = ground_cur;
		ground_points.clear();
	}
	else
	{
		auto entry = std::find_if(ground_points.begin(), ground_points.end(),
			[runway](Point2* point) { return pointOnPath(point, runway); });

		if (entry == ground_points.end())
			return false;

		runway_entry = *entry;
		ground_points.erase(entry + 1, ground_points.end());
	}

	Point2* runway_end = runway->getEnd();
	runway->getPoints(runway_entry, runway_end, ground_points);
	if (runway_end && runway_entry != runway_end &&
		(ground_points.empty() || ground_points.back() != runway_end))
	{
		ground_points.push_back(runway_end);
	}

	ground_next = ground_points.empty() ? nullptr : ground_points.front();
	ground_next_next = ground_points.size() > 1 ? ground_points[1] : nullptr;
	return true;
}

double RouteManager::calculateGS(const AircraftState& state, double thresh_lat, double thresh_lon, double threshelevation_ft, double capture_ft)
{
	double deltaLatNm = (state.latitude - thresh_lat) * 60.0;
	double deltaLonNm = (state.longitude - thresh_lon) * NauticalMilesPerDegreeLon(state.latitude);
	double requiredGSAltitudeFt = sqrt(deltaLonNm * deltaLonNm + deltaLatNm * deltaLatNm) * 6076.1 * tan(0.05235602094240838) + threshelevation_ft;// is the Runway Threshold Elevation
	double gsCaptureAlt = capture_ft;//Glide Slope capture altitude, example 2000 ft.
	double targetAltitudeFt = ((!(requiredGSAltitudeFt < gsCaptureAlt)) ? gsCaptureAlt : requiredGSAltitudeFt);
	if (targetAltitudeFt < state.altitude)
	{
		gsCaptureAlt = capture_ft;
		if (requiredGSAltitudeFt < gsCaptureAlt)
		{
			return requiredGSAltitudeFt;
		}
		return gsCaptureAlt;
	}
	return state.altitude;
}

double RouteManager::calculateLoc(const AircraftState& state, const AssignedValues& assigned, double dest_lat, double dest_lon, double loc_brg, double default_hdg, bool onGround)
{
	// Use the passed-in state object instead of member variables
	double turn_rate = onGround ? assigned.asdg_gnd_turn_rate : get_rot(state.roll, state.speed, 1000);; // Simplified for now, should come from assigned values
	double course = degrees(GetHeading(state.latitude, dest_lat, state.longitude, dest_lon));
	double d_lat = (state.latitude - dest_lat) * 60.0;
	double d_lon = (state.longitude - dest_lon) * NauticalMilesPerDegreeLon(state.latitude);
	double num5 = sqrt(d_lon * d_lon + d_lat * d_lat) * sin((course - loc_brg) * 0.017453277777777779);// * 0.017xxx is to radians
	double num6 = (double)state.speed * 0.033333333333333333 * 0.15915507752443828;
	double num7 = num6 - cos((state.heading - loc_brg) * 0.017453277777777779) * num6;
	double num8 = abs(num5);
	if (num8 < (double)((150 / turn_rate) * 0.00013888888888888889))//adjust this number to a lower value to adjust precision
	{
		return (int)(num5 * 8.0 + loc_brg + 720.0) % 360;
	}
	if (num7 < num8)// this is for checking how close we are to the bearing
	{
		return (double)default_hdg;//should be destination heading
	}
	return loc_brg;
}

double RouteManager::calculateGain(const AircraftState& state, const AssignedValues& assigned, Point2 cur, Point2 prev, double loc_brg, bool onGround)
{
	double course = degrees(GetHeading(state.latitude, cur.y_, state.longitude, cur.x_));
	double delta = get_angle_unsigned(loc_brg, course);

	double limit = 45;
	double gain = delta < 0 ? limit : delta > 0 ? -limit : 0;

	double heading = calculateLoc(state, assigned, cur.y_, cur.x_, loc_brg, hdg(loc_brg + gain), onGround);

	if (heading == ((int)loc_brg) && delta != 0)
	{
		heading = hdg(loc_brg + -delta);
	}

	return heading;
}

void RouteManager::updateNavigation(ACF_STATE& aircraft_state, AircraftState& state, AssignedValues& assigned, DefaultValues& defaults, const PerfValues& perf, Airport* airport) {
	if (aircraft_state == ACF_STATE::AIRBORNE) {
		// Handle airborne navigation
	}
	else { // On Ground
		if (ground_cur) {
			if (arrived(state, assigned, defaults)) {
				if (ground_cur) {
					state.MarkDirty(AircraftDirtyFlags::TRACK | AircraftDirtyFlags::DATA);
					AppendTextToConsole(L"[Arrived at : " +
						std::wstring(ground_cur->parent->name.begin(), ground_cur->parent->name.end()) +
						L" : " + std::to_wstring(ground_cur->index) + L"]");
					while (ground_route.size() > 0 && (ground_cur->parent->name != ground_route.front()))
						ground_route.erase(ground_route.begin());
				}
				pollRoute();
				if (!ground_cur) {
					aircraft_state = ACF_STATE::IDLE;
					state.MarkDirty(AircraftDirtyFlags::TRACK);
				}
			}
			else {
				if (!ground_prev) {
					// ... initial turn logic ...
					double brng = get_bearing(state.latitude, state.longitude, ground_cur->y_, ground_cur->x_);
					this->initialTurnAngle = fabs(state.heading - brng);
					if (this->initialTurnAngle > 180.0) {
						this->initialTurnAngle = 360.0 - this->initialTurnAngle;
					}
					const double SOME_SMALL_THRESHOLD = 5.0;
					if (fabs(state.heading - get_bearing(state.latitude, state.longitude, ground_cur->y_, ground_cur->x_)) < SOME_SMALL_THRESHOLD) {
						if (assigned.asdg_speed != defaults.speed)
							assigned.asdg_speed = defaults.speed;
						this->initialTurnAngle = -1.0;
					}
					else {
						double speedForTurn = calcSpeedForInitTurn(this->initialTurnAngle);
						assigned.asdg_speed = speedForTurn;
					}

					if (assigned.asgd_heading != brng) {
						assigned.asgd_heading = brng;
					}
				}
				else {
					double h = hdg(degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_)));
					double f_heading = calculateGain(state, assigned, *ground_cur, *ground_prev, h, onGround(aircraft_state));
					assigned.asgd_heading = hdg(f_heading);
				}
			}
			if (aircraft_state != ACF_STATE::TAKEOFF)
				checkRateReset(assigned, defaults, OnTrack(state));
		}
		if (aircraft_state != ACF_STATE::TAKEOFF)
			checkPathHolds(aircraft_state, state, assigned);
	}
}

bool RouteManager::onGround(ACF_STATE acf_state) const {
	return acf_state != ACF_STATE::AIRBORNE && acf_state != ACF_STATE::LANDING;
}


bool RouteManager::isTaxing() const {
	return ground_cur != nullptr;
}

bool RouteManager::isHoldingForTakeoff() const {
	return runway_ctx && HoldingDepart && runway_ctx == HoldingDepart;
}

double RouteManager::GetTrackTurnAngle(const AircraftState& state)
{
	if (ground_cur && ground_next)
	{
		double locBrg0 = ground_prev ? degrees(GetHeading(ground_prev->y_, ground_cur->y_, ground_prev->x_, ground_cur->x_))
			: degrees(GetHeading(state.latitude, ground_cur->y_, state.longitude, ground_cur->x_));
		double locBrg1 = degrees(GetHeading(ground_cur->y_, ground_next->y_, ground_cur->x_, ground_next->x_));

		double angle = get_angle(locBrg1, locBrg0);
		return angle;
	}
	return 0.0;
}

double RouteManager::GetTrackTurnData(const AircraftState& state)
{
	if (ground_cur && ground_next)
	{
		return CalcTaxiTurnRate(GetTrackTurnAngle(state));
	}
	return DEFAULT_TURN_RATE;
}

double RouteManager::GetTrackSpeedData(const AircraftState& state, const DefaultValues& defaults)
{
	if (ground_cur && ground_next)
	{
		return CalcTaxiSpeed(GetTrackTurnAngle(state), defaults.speed);
	}
	return defaults.speed;
}

double RouteManager::GetTurnLeadDistance(const AircraftState& state, const DefaultValues& defaults)
{
	if (!ground_cur || !ground_next)
		return 0.0;

	double angle = GetTrackTurnAngle(state);
	double turnRate = GetTrackTurnData(state);
	double trackSpeed = GetTrackSpeedData(state, defaults);
	double turnRadius = TurnRadius(trackSpeed, turnRate);
	return get_radius_of_turn(angle, turnRadius);
}

void RouteManager::updateTurnSpeedControl(const AircraftState& state, AssignedValues& assigned, const DefaultValues& defaults)
{
	if (!ground_cur || !ground_next)
		return;

	double trackSpeed = GetTrackSpeedData(state, defaults);
	if (assigned.asdg_speed <= trackSpeed)
		return;

	double leadDistance = GetTurnLeadDistance(state, defaults);
	double decelDistance = GetDecelerationDistance(state.speed, trackSpeed, assigned.asdg_gnd_braking);
	double distToPoint = GetDistance(state.latitude, state.longitude, ground_cur);

	if (distToPoint <= leadDistance + decelDistance)
	{
		assigned.asdg_speed = trackSpeed;
	}
}

bool RouteManager::OnTrack(const AircraftState& state)
{
	if (ground_cur && ground_prev)
	{
		double course = degrees(GetHeading(state.latitude, ground_cur->y_, state.longitude, ground_cur->x_));
		double locBrg = degrees(GetHeading(*ground_prev, *ground_cur));
		double delta = get_angle_unsigned(locBrg, course);
		double delta2 = get_angle_unsigned(locBrg, state.heading);
		return (delta < 1 && delta > -1) && (delta2 < 1 && delta2 > -1);
	}
	return false;
}

void RouteManager::checkRateReset(AssignedValues& assigned, DefaultValues& defaults, bool no_track)
{
	if (locked_rate)
	{
		if (no_track) // Simplified from OnTrack() for now
		{
			if (assigned.asdg_gnd_turn_rate != DEFAULT_TURN_RATE) {
				assigned.asdg_gnd_turn_rate = DEFAULT_TURN_RATE;
			}
			if (assigned.asdg_speed != defaults.speed) {
				assigned.asdg_speed = defaults.speed;
			}
			locked_rate = false;
		}
	}
}

void RouteManager::checkPathHolds(ACF_STATE& aircraft_state, const AircraftState& state, const AssignedValues& assigned)
{
	if (HoldingAt || HoldingDepart) return;
	if (aircraft_state == ACF_STATE::HOLDING) return;

	if (!holds.empty())
	{
		auto it = holds.begin();
		while (it != holds.end())
		{
			Point2* p = *it;
			double decel_distance0 = GetDecelerationDistance(state.speed, 0.0, assigned.asdg_gnd_braking);
			double dist = (300 / KNOTS_FT) + decel_distance0;
			if (circularDistance(state, p, (dist * KNOTS_KM) * 1000.0))
			{
				aircraft_state = ACF_STATE::HOLDING;
				it = holds.erase(it);
				if (p->parent)
				{
					HoldingAt = p->parent;
					if (runway_ctx && runway_ctx == p->parent)
					{
						HoldingDepart = p->parent;
					}
				}
				break; // Only process one hold per frame
			}
			++it;
		}
	}
}

bool RouteManager::arrived(AircraftState& state, AssignedValues& assigned, const DefaultValues& defaults)
{
	if (!ground_cur || !ground_next)
		return false;

	updateTurnSpeedControl(state, assigned, defaults);

	if (isTurnReady(state, defaults))
	{
		double new_turn_rate = GetTrackTurnData(state);
		if (assigned.asdg_gnd_turn_rate != new_turn_rate) {
			assigned.asdg_gnd_turn_rate = new_turn_rate;
			state.MarkDirty(AircraftDirtyFlags::DATA);
		}
		assigned.asdg_speed = GetTrackSpeedData(state, defaults);
		locked_rate = true;
		return true;
	}

	return defaultTurnDistance(state);
}

bool RouteManager::isTurnReady(const AircraftState& state, const DefaultValues& defaults)
{
	if (!ground_cur || !ground_next) return false;

	double leadDistance = GetTurnLeadDistance(state, defaults);

	return circularDistance(state, ground_cur, ((leadDistance * KNOTS_KM)) * 1000.0);
}

bool RouteManager::defaultTurnDistance(const AircraftState& state)
{
	if (!ground_cur) return false;
	return circularDistance(state, ground_cur, 2.2);
}

bool RouteManager::circularDistance(const AircraftState& state, Point2* p, double distance_meters)
{
	if (!p) return false;

	double interval_dist = get_distance(state.speed, 30); // Use constant for prediction
	Point2 n = getLocFromBearing(state.latitude, state.longitude, interval_dist, state.heading);

	Point2 v = getLocFromBearing(p->y_, p->x_, (distance_meters / 1000.0) / KNOTS_KM, 0);

	double num2 = (v.y_ - p->y_);
	double num3 = (v.x_ - p->x_);
	double radius = sqrt((num3 * num3) + (num2 * num2));

	return inCircle2(Point2(state.longitude, state.latitude), n, *p, radius);
}

void RouteManager::refreshRoute()
{
	if (!ground_next && ground_cur && ground_points.size() >= 2)
	{
		auto next = (ground_points.begin() + 1);
		if (next != ground_points.end()) ground_next = *next;
		if (next != ground_points.end() && (next + 1) != ground_points.end()) ground_next_next = *(next + 1);
	}
}

bool RouteManager::doPointSkip()
{
	if (ground_next && ground_next_next)
	{
		ground_points.erase(std::remove(ground_points.begin(), ground_points.end(), ground_next), ground_points.end());
		ground_next = nullptr;
		refreshRoute();
		return true;
	}
	return false;
}

double RouteManager::calcSpeedForInitTurn(double turnAngle) {
	const double MAX_INIT_SPEED = 10.0;
	if (turnAngle < 10) return MAX_INIT_SPEED;
	if (turnAngle < 45) return 0.75 * MAX_INIT_SPEED;
	if (turnAngle < 90) return 0.5 * MAX_INIT_SPEED;
	return 0.25 * MAX_INIT_SPEED;
}
