#pragma once

#include <chrono>

// Forward declarations to avoid circular dependencies
struct AircraftState;
struct AssignedValues;
struct PerfValues;
struct DefaultValues;

class FlightModel {
public:
    void updateState(AircraftState& state, const AssignedValues& assigned, const PerfValues& perf, const DefaultValues& defaults, bool onGround, bool isHolding, bool isTakeoff);

private:
    long long last_time_movement = 0;
    long long last_time_heading = 0;
    long long last_time_speed = 0;
    long long last_time_altitude = 0;
    long long last_time_roll = 0;
    long long last_time_pitch = 0;

    double getNextSpeed(double current_speed, const AssignedValues& assigned, const PerfValues& perf, bool onGround, bool isHolding, bool isTakeoff, double interval_ms);
    double getNextHeading(double current_heading, double current_roll, double current_speed, const AssignedValues& assigned, int turnOri, bool onGround, double interval_ms);
    double getNextPitch(double current_pitch, const AssignedValues& assigned, const PerfValues& perf, double interval_ms);
    double getNextRoll(double current_roll, const AssignedValues& assigned, const PerfValues& perf, double interval_ms);
    double getNextAltitude(double current_altitude, double current_vspeed, const AssignedValues& assigned, double interval_ms);
    void updateMovement(AircraftState& state, double interval_ms);
};