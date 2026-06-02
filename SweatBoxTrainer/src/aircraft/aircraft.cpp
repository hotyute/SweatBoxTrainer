#include "SweatBoxTrainer/aircraft/aircraft.h"
#include "SweatBoxTrainer/tools.h"
#include "SweatBoxTrainer/geoutils.h"
#include "SweatBoxTrainer/guidialogue.h"
#include "SweatBoxTrainer/packets_out.h"
#include "SweatBoxTrainer/usermanager.h"
#include "SweatBoxTrainer/tools/thread_pool.h"
#include "SweatBoxTrainer/globals.h"
#include "SweatBoxTrainer/sim/simulation_context.h"
#include <iostream>

namespace {
	bool routePointOnPath(Point2* point, TaxiPath* path)
	{
		return point && path && point->parent &&
			(point->parent == path || point->parent->name == path->name);
	}

	bool routeSegmentOnPath(Point2* from, Point2* to, TaxiPath* path)
	{
		return routePointOnPath(from, path) && routePointOnPath(to, path);
	}
}

Aircraft::Aircraft() : m_connection(this) {
	// Constructor logic if any
	std::fill_n(frequency, sizeof(frequency) / sizeof(frequency[0]), 99998);
}

Aircraft::~Aircraft() {
	stopPositionUpdates();
	stopPingUpdates();
}

void Aircraft::update() {
	try {
		// High-level update logic

		// 1. Update Physics
		m_flightModel.updateState(m_state, m_assignedValues, m_perfValues, m_defaultValues, onGround(), state == ACF_STATE::HOLDING, state == ACF_STATE::TAKEOFF);

		// 2. Update Navigation
		if (m_state.speed > 0 && state != ACF_STATE::HOLDING && state != ACF_STATE::IDLE) {
			m_routeManager.updateNavigation(state, m_state, m_assignedValues, m_defaultValues, m_perfValues, getAirport());
		}

		// 3. Check for collisions and other state changes
		CollisionDetection();
		CheckFrameFlags();
	}
	catch (const std::exception& e) {
		AppendTextToConsole(s2ws("Aircraft update error: " + std::string(e.what())));
		// Reset to safe state
		state = ACF_STATE::IDLE;
	}
	catch (...) {
		AppendTextToConsole(s2ws("Critical aircraft update error"));
		state = ACF_STATE::IDLE;
	}
}

Airport* Aircraft::getAirport() {
	if (apt_icao.empty() || apt_icao.length() < 4)
		return nullptr;

	std::string icao = apt_icao.substr(0, 4);
	auto& ctx = SimulationContext::instance();

	if (!airport || !ci_string_equal(airport->icao, icao)) {
		auto it = ctx.airports().find(icao);
		airport = (it != ctx.airports().end()) ? it->second.get() : nullptr;
	}
	return airport;
}

bool Aircraft::onGround() const {
	// This logic might need to be more robust, e.g. checking against airport elevation
	return m_state.altitude <= 0 || (airport && (m_state.altitude <= airport->elevation + 5));
}

void Aircraft::setMode(int newMode) {
	if (mode != newMode) {
		mode = newMode;
		m_state.MarkDirty(AircraftDirtyFlags::MODE);
	}
}

void Aircraft::setVerticalSpeed(double vs) {
	if (m_state.verticalSpeed != vs) {
		m_state.verticalSpeed = vs;
		m_state.MarkDirty(AircraftDirtyFlags::VSPEED);
	}
}

void Aircraft::CheckFrameFlags()
{
	if (onGround())
	{
		if (m_routeManager.OnTrack(m_state))
		{
			if (m_routeManager.queue_takeoff && m_routeManager.runway_ctx &&
				routeSegmentOnPath(m_routeManager.ground_prev, m_routeManager.ground_cur, m_routeManager.runway_ctx))
			{
				if (!m_routeManager.lineup)
				{
					handle_takeoff_roll();
					m_routeManager.queue_takeoff = false;
				}
				else
					state = ACF_STATE::HOLDING;
			}
			else if (state == ACF_STATE::TAKEOFF && m_state.speed >= m_perfValues.v1)
			{
				handle_takeoff_rotate();
			}
		}
	}
}

void Aircraft::CollisionDetection()
{
	if (!onGround()) return;

	auto nextLoc = GetNextLoc();
	auto currentAirport = getAirport();

	if (m_routeManager.HoldingFor)
	{
		Aircraft& other = *m_routeManager.HoldingFor;
		Point2 myPos(m_state.longitude, m_state.latitude);
		Point2 otherPos(other.getState().longitude, other.getState().latitude);
		double angle = get_angle(degrees(GetHeading(myPos, otherPos)), m_state.heading);
		if (angle > 90.0 || GetDistance(otherPos, myPos) > (300 / KNOTS_FT))
		{
			m_routeManager.HoldingFor = nullptr;
			if (!m_routeManager.HoldingAt && !m_routeManager.HoldingDepart)
				state = ACF_STATE::TAXING;
		}
	}
	else if (state != ACF_STATE::HOLDING)
	{
		auto& ctx = SimulationContext::instance();
		// The lock guard is not needed here. The caller (CalculateMovements) on same thread, handles it.

		if (ctx.aircraft().empty()) return;

		Aircraft* hold_for = nullptr;
		double last_distance = 0;
		double decel_distance0 = GetDecelerationDistance(m_state.speed, 0.0, m_assignedValues.asdg_gnd_braking);

		for (const auto& [callsign, otherPtr] : ctx.aircraft())
		{
			Aircraft* other = otherPtr.get();

			if (!other || other == this) continue;

			if (other->onGround() && other->getAirport() == currentAirport)
			{
				double cur_dist = GetDistance(other->GetNextLoc(), nextLoc);
				double dist = (300 / KNOTS_FT) + decel_distance0;
				if (cur_dist <= dist)
				{
					double angle = get_angle(degrees(GetHeading(nextLoc, other->GetNextLoc())), m_state.heading);
					if ((last_distance == 0.0 || cur_dist < last_distance) && angle <= 90.0)
					{
						// Simplified taxiway intersection logic
						if (m_routeManager.ground_cur && other->m_routeManager.ground_cur &&
							taxiIntersect(*m_routeManager.ground_cur, *other->m_routeManager.ground_cur))
						{
							hold_for = otherPtr.get();
							state = ACF_STATE::HOLDING;
							last_distance = cur_dist;
							AppendTextToConsole(s2ws(getCallSign() + ", holding for: " + other->getCallSign()));
						}
					}
				}
			}
		}
		m_routeManager.HoldingFor = hold_for;
	}
}

Point2 Aircraft::GetNextLoc()
{
	// This is predictive and should use the flight model
	// For now, a simplified version
	double next_heading = m_state.heading; // Not predicting turn
	double next_speed = m_state.speed; // Not predicting accel
	double interval_dist = get_distance(next_speed, 30); // 30ms prediction

	return getLocFromBearing(m_state.latitude, m_state.longitude, interval_dist, next_heading);
}

void Aircraft::handle_takeoff_roll()
{
	m_assignedValues.asdg_gnd_turn_rate = TURN_RATE_TAXI;
	m_routeManager.locked_rate = false;
	m_assignedValues.asdg_speed = m_perfValues.v1;
	state = ACF_STATE::TAKEOFF;
}

void Aircraft::handle_takeoff_rotate()
{
	m_routeManager.resetPath(m_assignedValues, m_defaultValues);
	m_routeManager.resetHolding();
	m_routeManager.resetContext();
	m_assignedValues.asdg_gnd_turn_rate = 10;
	m_assignedValues.asdg_speed = m_perfValues.climb;
	m_assignedValues.asdg_altitude = m_perfValues.init_alt;
	pass_standard_pitch(m_perfValues.init_alt);
	state = ACF_STATE::AIRBORNE;
}

void Aircraft::pass_standard_pitch(double init_alt)
{
	if (init_alt > m_assignedValues.asdg_altitude)
		m_assignedValues.asdg_pitch = m_perfValues.max_pitch_up;
	else if (init_alt < m_assignedValues.asdg_altitude)
		m_assignedValues.asdg_pitch = m_perfValues.max_pitch_down;
}


// These don't belong in Aircraft.cpp but are here for now until command processing is refactored
FlightPlan::FlightPlan()
{
	FlightPlan::squawkCode = "0000";
	FlightPlan::departure = "";
	FlightPlan::arrival = "";
	FlightPlan::alternate = "";
	FlightPlan::acType = "";
	FlightPlan::scratchPad = "";
	FlightPlan::cruise = "";
	FlightPlan::route = "";
	FlightPlan::remarks = "";
}

void Aircraft::startPositionUpdates(ThreadPool& pool)
{
	// If a task is already running, stop it first.
	stopPositionUpdates();

	// Create a new task specific to this aircraft with its server-defined interval
	if (update_time_interval_ms > 0) {
		m_positionUpdateTask = std::make_unique<AircraftPositionUpdateTask>(pool, *this);
		m_positionUpdateTask->start();
	}
}

void Aircraft::stopPositionUpdates()
{
	if (m_positionUpdateTask) {
		m_positionUpdateTask->stop();
		// The unique_ptr will handle deleting the object once it's out of scope
		// or when a new one is assigned.
		m_positionUpdateTask.reset();
	}
}

void Aircraft::startPingUpdates(ThreadPool& pool)
{
	// This logic ensures we don't create duplicate tasks.
	if (m_pingTask && m_pingTask->isRunning()) {
		return; // Already running, nothing to do.
	}

	// Create and start a new ping task for this aircraft.
	m_pingTask = std::make_unique<AircraftPingTask>(pool, *this);
	m_pingTask->start();
}

void Aircraft::stopPingUpdates()
{
	if (m_pingTask) {
		m_pingTask->stop();
		m_pingTask.reset(); // Destroys the task object.
	}
}

void Aircraft::disconnect(bool queue)
{
	if (connected) {
		closesocket(getConnection().sConnect);
		sendDisconnect(*this);
		getConnection().disconnect_socket();
		connected = false;

		// Clean up from context. This is called from tcp_manager::poll_socket, which
		// is inside the SocketPollingTask loop, so the mutex is already held.
		auto& ctx = SimulationContext::instance();
		if (this->userIndex != -1) {
			ctx.indexToCallsignMap().erase(this->userIndex);
		}
	}
	// IMPORTANT: Stop the task when the aircraft disconnects
	stopPositionUpdates();
	stopPingUpdates();
}
